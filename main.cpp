#include "bufanalysiswin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BufAnalysisWin w;
    w.show();

    return a.exec();
}
