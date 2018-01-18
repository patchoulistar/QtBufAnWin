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
    /*������Ҫ���ܵ��źŲ�*/
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
    FreeMessage();  //�ͷ��ڴ�
    delete ui;
}

void BufAnalysisWin::init() {
    /*��ʼ���������, ��ʼ����������ʽ*/
    inBuf = NULL;
    isHasMessage = false;
    isHasMessages = false;
    this->setWindowTitle(TITLESTR);

    model = new QStandardItemModel;
    model3 = new QStandardItemModel;
    model->setColumnCount(CLOUMNCOUNT);
    model3->setColumnCount(CLOUMNCOUNT2);
    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit(DESCRIPTORCODE));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit(POWER));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit(BASE));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit(WIDTH));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit(VALUE));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit(VALUES));
    model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit(VALUE2));
    model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit(UNIT));
    model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit(DESCRIBE));

    model3->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit(DESCRIPTORCODE));
    model3->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit(POWER));
    model3->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit(BASE));
    model3->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit(WIDTH));
    model3->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit(INFROWIDTH));
    model3->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit(DESCRIBE));

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    ui->tableView_2->setModel(model3);
    ui->tableView_2->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tabWidget->setStyleSheet("QTabWidget::pane {border:0; }");

    ui->sec3_edit->setReadOnly(true);
    ui->sec0_edit->setReadOnly(true);
    ui->sec1_edit->setReadOnly(true);
    ui->FileHeadEdit->setReadOnly(true);
    ui->tabWidget->setCurrentIndex(0);  //Ĭ��ѡ���һҳ
}

int getByt(QString number){
    /*�������ַ���תʮ�������ͺ���*/
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
    /*�����Ϣ������, ��ʾ�ڱ����*/
    BufAnalysisWin *se = (BufAnalysisWin*)pother;
    int i = 0;
    QList<QStandardItem*> items;
    if (pdes->code[0] == '1') {
        items.append(new QStandardItem(" "));
        items.append(new QStandardItem(QString(pdes->code)));
        items.append(new QStandardItem(QString::number(value[i], 10, 6)));
    }
    else if (pdes->code[0] == '0') {
        items.append(new QStandardItem(QString(pdes->code))); //����������
        items.append(new QStandardItem(QString::number(pdes->power))); //��������
        items.append(new QStandardItem(QString::number(pdes->base))); //��ֵ
        items.append(new QStandardItem(QString::number(pdes->width))); //���
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


int print_sec3(/*double *value, */DescriptorItem * pdes, void * pother){
    /*������ݶ�3�������*/
    BufAnalysisWin *se = (BufAnalysisWin*)pother;
    QList<QStandardItem*> items;
    items.append(new QStandardItem(QString(pdes->code))); //����������
    items.append(new QStandardItem(QString::number(pdes->power))); //��������
    items.append(new QStandardItem(QString::number(pdes->base))); //��ֵ
    items.append(new QStandardItem(QString::number(pdes->width))); //���
    items.append(new QStandardItem(QString::number(pdes->infrowidth)));  //�����ֶγ���
    items.append(new QStandardItem(QString(pdes->describe)));
    se->appendRow3(items);
    return 0;
}

int getWMO(double *value, int n, char * cval, int cvallen, DescriptorItem * pdes, int si, void * pother) {
    /*�˺���ֻ���ڻ�ȡʡ�ź�վ��,�õ������������������б�*/
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

void BufAnalysisWin::setInFileName(QString path){
    /*����Ҫ��ȡ�ļ���·��, �ж��Ƿ�Ϊ�涨�ĺ�׺*/
    if (path == "") {
        return;
    }
    QString sub = QFileInfo(path).suffix();
    if ((sub != "bin" && sub != "b") && (sub != "BIN" && sub != "B")) {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�ļ���ʽ����ȷ!"));
        return;
    }
    QByteArray paths = path.toLatin1();
    setsWindowTitle(QFileInfo(path).completeBaseName());
    inBuf = paths.data();
    cleartable(true);
    FreeMessage();
    emit send_sig_to_GetBufrMessage();
}

void BufAnalysisWin::OpenFileAction() {
    /*���ļ����·����,���ö�Ӧ������·����Ϊ�źŲ�������*/
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("�򿪵��ݱ����ļ�"), "", tr("AllFile(*.*);;b(*.b);;bin(*.bin)"));
    isFiles = false;
    emit send_sig_to_setInFileName(fileName);
}

void BufAnalysisWin::OpenFilesAction() {
    /*���ļ����·����,��·����Ϊ�źŲ�������*/
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("�򿪶�ݱ����ļ�"), "", tr("AllFile(*.*);;b(*.b);;bin(*.bin)"));
    isFiles = true;
    emit send_sig_to_setInFileName(fileName);
}

void BufAnalysisWin::GetBufrMessage() {
    /*���ýӿں���,�õ����ش�����, ����򿪵��Ƕ��������ļ����汨�����, �������, ���ж��ļ��Ĵ�����, Ȼ���͵��ò��ź�*/
    int ret = readmessage();
    if(isHasMessages){
        if (ret > 0) {
            msgcount = ret;
        }
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
    /*���ݴ򿪵İ�ť��ͬ, ���ò�ͬ�ķ����ӿں��������ض�Ӧ��ֵ*/
    bufr_init(NULL);
    if (!isFiles) {
        isHasMessage = true;
        isHasMessages = false;
        return bufrdec_file(inBuf, &bufrmsg);
    }
    else {
        isHasMessage = false;
        isHasMessages = true;
        return bufrdecs_file(inBuf, &bufrmsgs, &codecount);
    }
}

void BufAnalysisWin::DisplayInformation() {
    /*��������б�, ���Զ�ѡ���һ��(���ݱ����ļ�ֻ�е�һ��)*/
    process_bufrsec4(&(bufrmsg.sec4), &(bufrmsg.sec3), &getWMO, this);
    ui->listWidget->addItem(new QListWidgetItem(QString::number(AreaCode) + QString::number(PortalCode)));
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //������Ӧ�п�
    ui->listWidget->setCurrentRow(0);
}

void BufAnalysisWin::DisplayInformations() {
    /*���ݶ౨���ļ��������صķ���,������������б�*/
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

void BufAnalysisWin::AnalsisSec3(Section3Info *psec3){
    /*������ݶ�3���*/
    DescriptorItem * head = psec3->pdescriptorHead;
    for(int i = 0; i < psec3->descriptorCount; i++){
        print_sec3(head, this);
        head = head->next;
    }
    ui->sec3_edit->insertPlainText(QString("Length of section3: ") + QString::number(psec3->length) + QString(" bytes\n"));
    ui->sec3_edit->insertPlainText(QString("Subset count: ") + QString::number(psec3->subsetCount) + QString("\n"));
    ui->sec3_edit->insertPlainText(QString("Is observe? ") + QString::number(psec3->observe) + QString("\n"));
    ui->sec3_edit->insertPlainText(QString("Is compress? ") + QString::number(psec3->compress) + QString("\n"));
    ui->sec3_edit->insertPlainText(QString("Origin descriptors ") + QString::number(psec3->codeCount) + QString("\n"));
    for(int i = 0; i < psec3->observe; i++){
        ui->sec3_edit->insertPlainText(QString("[" + QString::number(i+1) +"] ") + &(psec3->originDescriptors[i]) + "\n");
    }
    ui->sec3_edit->insertPlainText(QString("Expanded descriptors ") + QString::number(psec3->descriptorCount) + QString("\n"));
}


void BufAnalysisWin::FillInformations(int Row) {
    /*����ձ������, �ٸ��ݱ�ʶ�����*/
    cleartable();
    if (isHasMessage) {
        process_bufrsec4(&(bufrmsg.sec4), &(bufrmsg.sec3), &modify_print_sec4, this);
        AnalsisSec3(&(bufrmsg.sec3));
        FillFileHead(bufrmsg.phead);
        FillSec0(bufrmsg.ksec0);
        FillSec1(bufrmsg.ksec1);
    }
    else {
        process_bufrsec4(&(bufrmsgs[Row].sec4), &(bufrmsgs[Row].sec3), &modify_print_sec4, this);
        AnalsisSec3(&(bufrmsgs[Row].sec3));
        FillFileHead(bufrmsgs[Row].phead);
        FillSec0(bufrmsgs[Row].ksec0);
        FillSec1(bufrmsgs[Row].ksec1);
    }
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //���ñ�ͷ�Ŀ��Ϊ(�������ݾ���)
}

void BufAnalysisWin::appendRow(QList<QStandardItem*>& items) {
    model->appendRow(items);
}

void BufAnalysisWin::appendRow3(QList<QStandardItem*>& items) {
    model3->appendRow(items);
}

void BufAnalysisWin::cleartable(bool isClearListWidget) {
    /*������ݿؼ�*/
    model->removeRows(0, model->rowCount());
    model3->removeRows(0, model3->rowCount());
    if(isClearListWidget){
        ui->listWidget->clear();
    }
    ui->FileHeadEdit->clear();
    ui->sec0_edit->clear();
    ui->sec1_edit->clear();
    ui->sec3_edit->clear();
}

void BufAnalysisWin::FreeMessage() {
    /*�ͷ�������Ϣ*/
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
    /*���Ϊ����, ֱ�ӵ��ÿ�ӿں��������txt�ļ���*/
    QString filename = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("���Ϊ"), "", tr("txt(*.txt);;AllFile(*.*)"));
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

void BufAnalysisWin::setsWindowTitle(QString FileName){
    /*���ô��ڱ���*/
    if(isFiles){
        this->setWindowTitle(QString::fromLocal8Bit(TITLESTR) + QString::fromLocal8Bit("-") + QString::fromLocal8Bit(MULTIPARTITE) + QString::fromLocal8Bit(": ") + FileName);
    }else{
        this->setWindowTitle(QString::fromLocal8Bit(TITLESTR) + QString::fromLocal8Bit("-") + QString::fromLocal8Bit(SINGE) + QString::fromLocal8Bit(": ") + FileName);
    }

}

void BufAnalysisWin::FillFileHead(HeadInfo * phead){
       ui->FileHeadEdit->insertPlainText(QString("Length of file head: " + QString::number(phead->headlen) + " bytes\n"));
       ui->FileHeadEdit->insertPlainText(QString("Length of message: " + QString::number(phead->msglen) + "bytes\n"));
       ui->FileHeadEdit->insertPlainText(QString("Format: " + QString::number(phead->format) + "\n"));
       ui->FileHeadEdit->insertPlainText(QString("Sequence number: " + QString::number(phead->seqnum) + "\n"));
       ui->FileHeadEdit->insertPlainText(QString("TTAAii: " + QString((char*)phead->ttaaii) + "\n"));
       ui->FileHeadEdit->insertPlainText(QString("CCCC: " + QString((char*)phead->cccc) + "\n"));
       ui->FileHeadEdit->insertPlainText(QString("YYGGgg: " + QString((char*)phead->yygg) + "\n"));
       ui->FileHeadEdit->insertPlainText(QString("BBB: " + QString((char*)phead->bbb)));
}

void BufAnalysisWin::FillSec0(int * sec0){
    ui->sec0_edit->insertPlainText(QString("Length of section0: " + QString::number(sec0[0]) + " bytes\n"));
    ui->sec0_edit->insertPlainText(QString("Total length of Bufr message: " + QString::number(sec0[1]) + " bytes\n"));
    ui->sec0_edit->insertPlainText(QString("Bufr edition number: " + QString::number(sec0[2]) + "\n"));
}

void BufAnalysisWin::FillSec1(int * sec1){
    ui->sec1_edit->insertPlainText(QString("Lenth of section1: " + QString::number(sec1[0]) + " bytes\n"));
    ui->sec1_edit->insertPlainText(QString("Flag of BUFR master table: " + QString::number(sec1[1]) + " bytes\n"));
    ui->sec1_edit->insertPlainText(QString("Originating center: " + QString::number(sec1[2]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Originating sub_cnter: " + QString::number(sec1[3]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Update sequence number: " + QString::number(sec1[4]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Flag (presence of Section 2 in the message): " + QString::number(sec1[5]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Bufr message type (Bufr Table A): " + QString::number(sec1[6]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Bufr message sub-type (international): " + QString::number(sec1[7]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Bufr message sub-type (local): " + QString::number(sec1[8]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Version number of Master table used: " + QString::number(sec1[9]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Version number of Master local used: " + QString::number(sec1[10]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Year: " + QString::number(sec1[11]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Month: " + QString::number(sec1[12]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Day: " + QString::number(sec1[13]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Hour: " + QString::number(sec1[14]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Minute: " + QString::number(sec1[15]) + "\n"));
    ui->sec1_edit->insertPlainText(QString("Second: " + QString::number(sec1[16]) + "\n"));

}
