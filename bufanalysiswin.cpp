#include "bufanalysiswin.h"
#include "ui_bufanalysiswin.h"
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <bitset>

BufAnalysisWin::BufAnalysisWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BufAnalysisWin)
{
    ui->setupUi(this);
    init();
    connect(ui->OpenFile, &QAction::triggered, this, &BufAnalysisWin::OpenFileAction);
    connect(ui->OpenFiles, &QAction::triggered, this, &BufAnalysisWin::OpenFilesAction);
    connect(this, SIGNAL(send_sig_to_setInFileName(QString)), this, SLOT(setInFileName(QString)));
    connect(this, SIGNAL(send_sig_to_DisplayInformation()), this, SLOT(DisplayInformation()));
    connect(this, SIGNAL(send_sig_to_DisplayInformations()), this, SLOT(DisplayInformations()));
    connect(this, SIGNAL(send_sig_to_ResizesColumn()), ui->tableView, SLOT(resizeColumnsToContents()));
    connect(this, SIGNAL(send_sig_to_GetBufrMessage()), this, SLOT(GetBufrMessage()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(FillInformations(int)));
    connect(ui->save_as, &QAction::triggered, this, &BufAnalysisWin::FileSaveAS);
    connect(ui->quit, &QAction::triggered, this, &BufAnalysisWin::close);
}

BufAnalysisWin::~BufAnalysisWin()
{
    FreeMessage();
    delete ui;
}

void BufAnalysisWin::init() {
    inBuf = NULL;
    isHasMessage = false;
    isHasMessages = false;
    model = new QStandardItemModel;
    model->setColumnCount(CLOUMNCOUNT);
    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit(ELEMENT));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit(DESCRIPTORCODE));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit(POWER));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit(BASE));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit(WIDTH));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit(VALUE));
    model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit(VALUES));
    model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit(VALUE2));
    model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit(UNIT));
    model->setHeaderData(9, Qt::Horizontal, QString::fromLocal8Bit(DESCRIBE));
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
}

int getByt(QString number){
    if(number == NULL){
        return 0;
    }
    unsigned int i = 0;
    const char *pch = number.toStdString().c_str();
    while(*pch == '0' || *pch == '1'){
        i <<= 1;
        i |= *pch++ - '0';
    }
    return (int)i;
}


int modify_print_sec4(double *value, int n, char * cval, int cvallen, DescriptorItem * pdes, int si, void * pother){
    BufAnalysisWin *se = (BufAnalysisWin*)pother;
    int i = 0;
    QList<QStandardItem*> items;
    if (pdes->code[0] == '1') {
        items.append(new QStandardItem(" "));
        items.append(new QStandardItem(QString(pdes->code)));
        items.append(new QStandardItem(QString::number(value[i], 10, 6)));
    }
    else if (pdes->code[0] == '0') {
        items.append(new QStandardItem(QString::number(n))); //Ԫ����
        items.append(new QStandardItem(QString(pdes->code))); //����������
        items.append(new QStandardItem(QString::number(pdes->power))); //��������
        items.append(new QStandardItem(QString::number(pdes->base))); //��ֵ
        items.append(new QStandardItem(QString::number(pdes->width))); //����
        if (QString::fromLocal8Bit(pdes->code) == "001192" && cval == NULL) {  //�����ͬ��ʽվ���ļ���ʽ��BUG
            items.append(new QStandardItem(""));
        }
        if (n > 1) {
            QString x= QString::number((int)value[i], 2);
            if(x.size() == 8){
                int ll = getByt(x.left(4));
                int rr = getByt(x.right(4));
                items.append(new QStandardItem(QString::number(ll)));  //ֵ
                items.append(new QStandardItem(QString::number(rr)));  //ֵ
            }else{
                int rr = getByt(x.right(4));
                int ll = getByt(x.split(x.right(4))[0]);
                items.append(new QStandardItem(QString::number(ll)));  //ֵ
                items.append(new QStandardItem(QString::number(rr)));  //ֵ
            }
            value[i] = 1;
            i++;
        }else{
            items.append(new QStandardItem(""));
        }
        if (cval != NULL) {
            char * df = "";
            if (*cval != *df) {
                items.append(new QStandardItem(""));
            }
            items.append(new QStandardItem(QString::fromLocal8Bit(cval))); //ֵ
        }
        else {
            if (fabs(pow(2, pdes->width) - 1 - value[i] < 10e-6)) {
                if (n == 1) {
                    items.append(new QStandardItem(""));
                }
                items.append(new QStandardItem(QString(DEFAULTVALUE)));  //ֵ
            }
            else {
                if (n == 1) {
                    items.append(new QStandardItem(""));
                }
                items.append(new QStandardItem(QString::number(value[i], 10, 6))); //ֵ
            }
        }
        i++;
        items.append(new QStandardItem(QString(pdes->unit)));
        items.append(new QStandardItem(QString(pdes->describe)));
    }
    se->appendRow(items);
    return 0;
}

int getWMO(double *value, int n, char * cval, int cvallen, DescriptorItem * pdes, int si, void * pother) {
    BufAnalysisWin *se = (BufAnalysisWin*)pother;
    if (QString::fromLocal8Bit(pdes->code) == "001001") {
        se->setAreaCode(int(*value));
    }
    if (QString::fromLocal8Bit(pdes->code) == "001002") {
        se->setPortalCode(int(*value));
        return -6;
    }
    return 0;
}

void BufAnalysisWin::setInFileName(QString path) {
    if (path == "") {
        return;
    }
    QString sub = QFileInfo(path).suffix();
    if ((sub != "bin" && sub != "b") && (sub != "BIN" && sub != "B")) {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�ļ���ʽ����ȷ!"));
        return;
    }
    QByteArray paths = path.toLatin1();
    inBuf = paths.data();
    cleartable();
    FreeMessage();
    emit send_sig_to_GetBufrMessage();
}

void BufAnalysisWin::OpenFileAction() {
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("�򿪵��ݱ����ļ�"), "", tr("AllFile(*.*);;b(*.b);;bin(*.bin)"));
    isFiles = false;
    emit send_sig_to_setInFileName(fileName);
}

void BufAnalysisWin::OpenFilesAction() {
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("�򿪶�ݱ����ļ�"), "", tr("AllFile(*.*);;b(*.b);;bin(*.bin)"));
    isFiles = true;
    emit send_sig_to_setInFileName(fileName);
}

void BufAnalysisWin::GetBufrMessage() {
    int ret = readmessage();
    if (ret > 0) {
        msgcount = ret;
    }
    if (ret < IGNORE) {
        QMessageBox::warning(this, QString::fromLocal8Bit("���ļ�ʧ��"), QString::fromLocal8Bit("�������: " + ret));
        FreeMessage();
        return;
    }
    if (isFiles) {
        emit send_sig_to_DisplayInformations();
    }
    else {
        emit send_sig_to_DisplayInformation();
    }
}

int BufAnalysisWin::readmessage() {
    bufr_init(NULL);
    if (!isFiles) {
        isHasMessage = true;
        return bufrdec_file(inBuf, &bufrmsg);
    }
    else {
        isHasMessages = true;
        return bufrdecs_file(inBuf, &bufrmsgs, &codecount);
    }
}

void BufAnalysisWin::DisplayInformation() {
    process_bufrsec4(&(bufrmsg.sec4), &(bufrmsg.sec3), &getWMO, this);
    ui->listWidget->addItem(new QListWidgetItem(QString::number(AreaCode) + QString::number(PortalCode)));
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //������Ӧ�п�
    ui->listWidget->setCurrentRow(0);
}

void BufAnalysisWin::DisplayInformations() {
    for (int i = 1; i <= msgcount; i++) {
        process_bufrsec4(&(bufrmsgs[i - 1].sec4), &(bufrmsgs[i - 1].sec3), &getWMO, this);
        if (codecount[i-1] < IGNORE) {
            ui->listWidget->addItem(new QListWidgetItem("err code : " + QString::number(codecount[i-1])));
        }
        else {
            ui->listWidget->addItem(new QListWidgetItem(QString::number(AreaCode) + QString::number(PortalCode)));
        }
    }
    ui->listWidget->setCurrentRow(0);
}

void BufAnalysisWin::FillInformations(int Row) {
    model->removeRows(0, model->rowCount());
    if (isHasMessage) {
        process_bufrsec4(&(bufrmsg.sec4), &(bufrmsg.sec3), &modify_print_sec4, this);
    }
    else {
        process_bufrsec4(&(bufrmsgs[Row].sec4), &(bufrmsgs[Row].sec3), &modify_print_sec4, this);
    }
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //������Ӧ�п�
}

void BufAnalysisWin::appendRow(QList<QStandardItem*>& items) {
    model->appendRow(items);
}

void BufAnalysisWin::cleartable() {
    model->removeRows(0, model->rowCount());
    ui->listWidget->clear();
}

void BufAnalysisWin::FreeMessage() {
    if (isHasMessage) {
        free_bufrmsg(&bufrmsg);
        isHasMessage = false;
    }
    if (isHasMessages) {
        free_bufrmsgs(bufrmsgs, codecount, msgcount);
        isHasMessages = false;
    }
    bufr_end();
}

void BufAnalysisWin::setAreaCode(int code) {
    AreaCode = code;
}

void BufAnalysisWin::setPortalCode(int code){
    PortalCode = code;
}

void BufAnalysisWin::FileSaveAS() {
    QString filename = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("����Ϊ"), "", tr("txt(*.txt);;AllFile(*.*)"));
    if (filename.isEmpty()) {
        return;
    }
    else {
        QByteArray path = filename.toLatin1();
        if (isHasMessage) {
            print_bufrmsg(&bufrmsg, path.data());
        }
        else {
            print_bufrmsgs(bufrmsgs, msgcount, path.data());
        }
    }
}