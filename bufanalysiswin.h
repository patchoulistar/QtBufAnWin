#ifndef BUFANALYSISWIN_H
#define BUFANALYSISWIN_H

#include <QMainWindow>
#include <QStandardItemModel>
extern "C" {
#include <bufr.h>
}

#define CLOUMNCOUNT 10
#define ELEMENT "元素数"
#define DESCRIPTORCODE "描述符代码"
#define POWER "比例因子"
#define BASE "基值"
#define WIDTH "宽度"
#define VALUE "省级质量控制码"
#define VALUES "台站质量控制码"
#define VALUE2 "实际值"
#define UNIT "单位"
#define DESCRIBE "描述"

#define DEFAULTVALUE "MISSING"

#define TITLESTR "BufAnWin"
#define SINGE "单份报表"
#define MULTIPARTITE "多份报表"


namespace Ui {
class BufAnalysisWin;
}

class BufAnalysisWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit BufAnalysisWin(QWidget *parent = 0);
    ~BufAnalysisWin();
    void OpenFileAction();
    void OpenFilesAction();
    void appendRow(QList<QStandardItem*>&);
    void cleartable();
    int readmessage();
    void FreeMessage();
    void setAreaCode(int);
    void setPortalCode(int);
    void setsWindowTitle(QString);
private:
    void init();
    Ui::BufAnalysisWin *ui;
    char * inBuf;
    BufrMessage bufrmsg;
    BufrMessage *bufrmsgs;
    int *codecount;
    QStandardItemModel * model;
    bool isFiles;
    int msgcount;
    bool isHasMessage;
    bool isHasMessages;
    int AreaCode;
    int PortalCode;
signals:
    void send_sig_to_setInFileName(QString);
    void send_sig_to_DisplayInformation();
    void send_sig_to_DisplayInformations();
    void send_sig_to_ResizesColumn();
    void send_sig_to_GetBufrMessage();
public slots:
    void setInFileName(QString);
    void DisplayInformation();
    void DisplayInformations();
    void GetBufrMessage();
    void FillInformations(int);
    void FileSaveAS();
};

#endif // BUFANALYSISWIN_H
