#include <QApplication>

#include <ve_qitem_table_widget.hpp>
#include <velib/platform/plt.h>
#include <velib/platform/task_qt.hpp>
#include <velib/qt/ve_qitems_from_c.hpp>
#include <velib/types/ve_values.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TaskQt mTask;

    VeQItem *mRoot = VeQItems::getRoot();
    mRoot->setId("Root");
    VeItem *rootItem = veValueTree();
    VeQItemsFromC *provider = new VeQItemsFromC(mRoot, "Provider", rootItem);
    provider->open();

    VeQItemTableWidget table(mRoot);
    table.show();
    //provider->services()->itemGetOrCreate("Test/Unsigned32");

    mTask.start();

    return a.exec();
}
