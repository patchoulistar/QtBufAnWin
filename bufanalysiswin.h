#ifndef BUFANALYSISWIN_H
#define BUFANALYSISWIN_H

#include <QMainWindow>
#include <QStandardItemModel>
extern "C" {
#include <bufr.h>
}

#define CLOUMNCOUNT 10
#define ELEMENT "Ԫ����"
#define DESCRIPTORCODE "����������"
#define POWER "��������"
#define BASE "��ֵ"
#define WIDTH "����"
#define VALUE "ʡ������������"
#define VALUES "̨վ����������"
#define VALUE2 "ʵ��ֵ"
#define UNIT "��λ"
#define DESCRIBE "����"

#define DEFAULTVALUE "MISSING"

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