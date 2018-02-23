// Simple listener which outputs all received messages to the terminal in a machine-readable format

#include <iomanip>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <Defs.h>
#include <Driver.h>
#include <Group.h>
#include <Manager.h>
#include <Node.h>
#include <Notification.h>
#include <Options.h>
#include <platform/Log.h>
#include <value_classes/Value.h>
#include <value_classes/ValueBool.h>
#include <value_classes/ValueID.h>
#include <value_classes/ValueStore.h>

using std::string;
using std::cout;
using std::cerr;
using OpenZWave::ValueID;
using OpenZWave::Manager;
using OpenZWave::Options;
using OpenZWave::Notification;

static const string           defaultDriver = "/dev/ttyACM0";
static volatile bool          g_initFailed = false;
static volatile sig_atomic_t  g_done = false;
static volatile sig_atomic_t  g_signum = 0;
static pthread_mutex_t        g_criticalSection;
static pthread_cond_t         initCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t        initMutex = PTHREAD_MUTEX_INITIALIZER;

void PrintJsonString(string s) {
    cout << "\"";
    for (string::iterator c = s.begin(); c != s.end(); c++) {
        switch (*c) {
            case '"': cout << "\\\""; break;
            case '\\': cout << "\\\\"; break;
            case '\b': cout << "\\b"; break;
            case '\f': cout << "\\f"; break;
            case '\n': cout << "\\n"; break;
            case '\r': cout << "\\r"; break;
            case '\t': cout << "\\t"; break;
            default:
                if ('\x00' <= *c && *c <= '\x1f') {
                    cout << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int) *c;
                }
                else {
                    cout << *c;
                }
                break;
        }
    }
    cout << "\"";
}

/** Print the data for the given ValueID as JSON to the standard output */
void PrintValueID(uint32 h, uint8 n, ValueID v) {
    cout << "{\"homeId\":" << +h << ",\"nodeId\":" << +n;
    cout << ",\"label\":"; PrintJsonString(Manager::Get()->GetValueLabel(v));
    cout << ",\"type\":" << v.GetType();
    cout << ",\"help\":"; PrintJsonString(Manager::Get()->GetValueHelp(v));
    cout << ",\"units\":"; PrintJsonString(Manager::Get()->GetValueUnits(v));
    cout << ",\"min\":" << Manager::Get()->GetValueMin(v);
    cout << ",\"max\":" << Manager::Get()->GetValueMax(v);

    // 0 = ValueType_Bool        Boolean, true or false
    // 1 = ValueType_Byte        8-bit unsigned value
    // 2 = ValueType_Decimal     Represents a non-integer value as a string, to avoid floating point accuracy issues.
    // 3 = ValueType_Int         32-bit signed value
    // 4 = ValueType_List        List from which one item can be selected
    // 5 = ValueType_Schedule    Complex type used with the Climate Control Schedule command class
    // 6 = ValueType_Short       16-bit signed value
    // 7 = ValueType_String      Text string
    // 8 = ValueType_Button      A write-only value that is the equivalent of pressing a button to send a command to a device
    // 9 = ValueType_Raw         A collection of bytes

    switch (v.GetType()) {
        case ValueID::ValueType_Bool:
        {
            bool value;
            Manager::Get()->GetValueAsBool(v, &value);
            cout << ",\"value\":" << (value ? "true" : "false");
            break;
        }

        case ValueID::ValueType_Decimal:
        {
            string value;
            Manager::Get()->GetValueAsString(v, &value);
            cout << ",\"value\":" << value;
            break;
        }

        case ValueID::ValueType_String:
        {
            string value;
            Manager::Get()->GetValueAsString(v, &value);
            cout << ",\"value\":"; PrintJsonString(value);
            break;
        }

        case ValueID::ValueType_Byte:
        {
            uint8 value;
            Manager::Get()->GetValueAsByte(v, &value);
            cout << ",\"value\":" << +value;
            break;
        }

        case ValueID::ValueType_Short:
        {
            int16 value;
            Manager::Get()->GetValueAsShort(v, &value);
            cout << ",\"value\":" << +value;
            break;
        }

        case ValueID::ValueType_Int:
        {
            int32 value;
            Manager::Get()->GetValueAsInt(v, &value);
            cout << ",\"value\":" << +value;
            break;
        }

        default:
        {
        }
    }
    cout << "}\n";
}

/** Callback that is triggered when a value, group or node changes */
void OnNotification(Notification const* _notification, void* _context) {
    pthread_mutex_lock(&g_criticalSection);
    if (!g_done) switch (_notification->GetType()) {
        case Notification::Type_ValueAdded:
        case Notification::Type_ValueChanged:
        {
            PrintValueID(_notification->GetHomeId(), _notification->GetNodeId(), _notification->GetValueID());
            break;
        }

        case Notification::Type_ValueRemoved:
        {
            // TODO: Notify of deletion
            break;
        }

        case Notification::Type_Group:
        {
            // One of the node's association groups has changed
            break;
        }

        case Notification::Type_NodeAdded:
        {
            // Add the new node to our list
            // TODO: Print general info about device
            break;
        }

        case Notification::Type_NodeRemoved:
        {
            // Remove the node from our list
            // TODO: Notify of deletion
            break;
        }

        case Notification::Type_NodeEvent:
        {
            // Received an event from the node, caused by a basic_set or hail message
            break;
        }

        case Notification::Type_DriverFailed:
        {
            g_initFailed = true;
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_AwakeNodesQueried:
        case Notification::Type_AllNodesQueried:
        case Notification::Type_AllNodesQueriedSomeDead:
        {
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_PollingDisabled:
        case Notification::Type_PollingEnabled:
        case Notification::Type_DriverReady:
        case Notification::Type_DriverReset:
        case Notification::Type_Notification:
        case Notification::Type_NodeNaming:
        case Notification::Type_NodeProtocolInfo:
        case Notification::Type_NodeQueriesComplete:
        default:
        {
        }
    }
    pthread_mutex_unlock(&g_criticalSection);
}


void HandleSignal(int signum) {
    g_signum = signum;
    g_done = true;
    pthread_cond_broadcast(&initCond);

    // Only catch one, second signal causes default action (kill)
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = SIG_DFL;
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

int main(int argc, char* argv[]) {
    int opt;
    bool verbose = false;
    bool continueous = false;
    while ((opt = getopt(argc, argv, "cvh")) != -1) {
        switch (opt) {
            case 'c':
                continueous = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
            case '?':
                cerr << "Usage: " << argv[0] << " [-cv] [driver...]\n"
                    << "  -c   Continueous mode, keep listening indefinitely for messages\n"
                    << "  -v   Verbose mode, prints full status messages from the OpenZWave stack\n";
                return 1;
        }
    }

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = HandleSignal;
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    pthread_mutex_lock(&initMutex);

    Options::Create("config/", "", "");
    Options::Get()->AddOptionBool("ConsoleOutput", verbose);
    Options::Get()->Lock();

    Manager::Create();
    Manager::Get()->AddWatcher(OnNotification, NULL);

    // Add drivers
    if (argc > optind) {
        for (int i = optind; i < argc; i++) {
            Manager::Get()->AddDriver(argv[i]);
        }
    }
    else {
        Manager::Get()->AddDriver(defaultDriver);
    }

    pthread_cond_wait(&initCond, &initMutex);
    if (g_initFailed) {
        cerr << "Failed to initialize.";
    }
    else if (continueous) {
        while (!g_done) {
            pause();
        }
    }

    pthread_mutex_lock(&g_criticalSection);
    g_done = true;
    pthread_mutex_unlock(&g_criticalSection);

    if (argc > optind) {
        for (int i = argc - 1; i >= optind; i--) {
            Manager::Get()->RemoveDriver(argv[i]);
        }
    }
    else {
        Manager::Get()->RemoveDriver(defaultDriver);
    }
    Manager::Get()->RemoveWatcher(OnNotification, NULL);
    Manager::Destroy();
    Options::Destroy();
    pthread_mutex_destroy(&g_criticalSection);

    if (g_signum != 0) {
        kill(getpid(), g_signum);
    }
    else {
        return g_initFailed;
    }
}
