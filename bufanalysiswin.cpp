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
    /*绑定所需要功能的信号槽*/
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
    FreeMessage();  //释放内存
    delete ui;
}

void BufAnalysisWin::init() {
    /*初始化各项变量, 初始化两个表格格式*/
    inBuf = NULL;
    isHasMessage = false;
    isHasMessages = false;
    this->setWindowTitle(TITLESTR);

    model = new QStandardItemModel;
    model3 = new QStandardItemModel;
    model->setColumnCount(CLOUMNCOUNT);
    model3->setColumnCount(CLOUMNCOUNT2);
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

    model3->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit(DESCRIPTORCODE));
    model3->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit(POWER));
    model3->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit(BASE));
    model3->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit(WIDTH));
    model3->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit(VALUE2));
    model3->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit(DESCRIBE));

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    ui->tableView_2->setModel(model3);
    ui->tableView_2->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tabWidget->setStyleSheet("QTabWidget::pane {border:0; }");
    ui->tabWidget->setCurrentIndex(0);  //默认选择第一页
}

int getByt(QString number){
    /*二进制字符串转十进制整型函数*/
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
    /*库的信息处理函数, 显示在表格上*/
    BufAnalysisWin *se = (BufAnalysisWin*)pother;
    int i = 0;
    QList<QStandardItem*> items;
    if (pdes->code[0] == '1') {
        items.append(new QStandardItem(" "));
        items.append(new QStandardItem(QString(pdes->code)));
        items.append(new QStandardItem(QString::number(value[i], 10, 6)));
    }
    else if (pdes->code[0] == '0') {
        items.append(new QStandardItem(QString::number(n))); //元素数
        items.append(new QStandardItem(QString(pdes->code))); //描述符代码
        items.append(new QStandardItem(QString::number(pdes->power))); //比例因子
        items.append(new QStandardItem(QString::number(pdes->base))); //基值
        items.append(new QStandardItem(QString::number(pdes->width))); //宽度
        if (QString::fromLocal8Bit(pdes->code) == "001192" && cval == NULL) {  //解决不同格式站点文件格式的BUG
            items.append(new QStandardItem(""));
        }
        if (n > 1) {
            QString x= QString::number((int)value[i], 2);
            if(x.size() == 8){
                int ll = getByt(x.left(4));
                int rr = getByt(x.right(4));
                items.append(new QStandardItem(QString::number(ll)));  //值
                items.append(new QStandardItem(QString::number(rr)));  //值
            }else{
                int rr = getByt(x.right(4));
                int ll = getByt(x.split(x.right(4))[0]);
                items.append(new QStandardItem(QString::number(ll)));  //值
                items.append(new QStandardItem(QString::number(rr)));  //值
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
            items.append(new QStandardItem(QString::fromLocal8Bit(cval))); //值
        }
        else {
            if (fabs(pow(2, pdes->width) - 1 - value[i] < 10e-6)) {
                if (n == 1) {
                    items.append(new QStandardItem(""));
                }
                items.append(new QStandardItem(QString(DEFAULTVALUE)));  //值
            }
            else {
                if (n == 1) {
                    items.append(new QStandardItem(""));
                }
                items.append(new QStandardItem(QString::number(value[i], 10, 6))); //值
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
    /*填充数据段3表格内容*/
    BufAnalysisWin *se = (BufAnalysisWin*)pother;
    QList<QStandardItem*> items;
    items.append(new QStandardItem(QString(pdes->code))); //描述符代码
    items.append(new QStandardItem(QString::number(pdes->power))); //比例因子
    items.append(new QStandardItem(QString::number(pdes->base))); //基值
    items.append(new QStandardItem(QString::number(pdes->width))); //宽度
    items.append(new QStandardItem(""));  //可能是值  备用
    items.append(new QStandardItem(QString(pdes->describe)));
    se->appendRow3(items);
    return 0;
}

int getWMO(double *value, int n, char * cval, int cvallen, DescriptorItem * pdes, int si, void * pother) {
    /*此函数只用于获取省号和站号,得到的数据用来填充左侧列表*/
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
    /*保存要读取文件的路径, 判断是否为规定的后缀*/
    if (path == "") {
        return;
    }
    QString sub = QFileInfo(path).suffix();
    if ((sub != "bin" && sub != "b") && (sub != "BIN" && sub != "B")) {
        QMessageBox::warning(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("文件格式不正确!"));
        return;
    }
    QByteArray paths = path.toLatin1();
    setsWindowTitle(QFileInfo(path).completeBaseName());
    inBuf = paths.data();
    cleartable();
    FreeMessage();
    emit send_sig_to_GetBufrMessage();
}

void BufAnalysisWin::OpenFileAction() {
    /*打开文件获得路径名,设置对应变量将路径作为信号参数发出*/
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开单份报表文件"), "", tr("AllFile(*.*);;b(*.b);;bin(*.bin)"));
    isFiles = false;
    emit send_sig_to_setInFileName(fileName);
}

void BufAnalysisWin::OpenFilesAction() {
    /*打开文件获得路径名,将路径作为信号参数发出*/
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开多份报表文件"), "", tr("AllFile(*.*);;b(*.b);;bin(*.bin)"));
    isFiles = true;
    emit send_sig_to_setInFileName(fileName);
}

void BufAnalysisWin::GetBufrMessage() {
    /*调用接口函数,得到返回错误码, 如果打开的是多个报表的文件保存报表份数, 如果不是, 则判断文件的错误码, 然后发送调用槽信号*/
    int ret = readmessage();
    if(isHasMessage){
        if (ret > 0) {
            msgcount = ret;
        }
    }
    if (ret < IGNORE) {
        QMessageBox::warning(this, QString::fromLocal8Bit("打开文件失败"), QString::fromLocal8Bit("错误代码: " + ret));
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
    /*根据打开的按钮不同, 调用不同的分析接口函数并返回对应的值*/
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
    /*填充左则列表, 并自动选择第一项(单份报表文件只有第一项)*/
    process_bufrsec4(&(bufrmsg.sec4), &(bufrmsg.sec3), &getWMO, this);
    ui->listWidget->addItem(new QListWidgetItem(QString::number(AreaCode) + QString::number(PortalCode)));
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //列自适应列宽
    ui->listWidget->setCurrentRow(0);
}

void BufAnalysisWin::DisplayInformations() {
    /*根据多报表文件解析返回的份数,遍历填充左则列表*/
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
    /*填充数据段3表格*/
    DescriptorItem * head = psec3->pdescriptorHead;
    for(int i = 0; i < psec3->descriptorCount; i++){
        print_sec3(head, this);
        head = head->next;
    }
}


void BufAnalysisWin::FillInformations(int Row) {
    /*先清空表格内容, 再根据标识填充表格*/
    model->removeRows(0, model->rowCount());
    if (isHasMessage) {
        process_bufrsec4(&(bufrmsg.sec4), &(bufrmsg.sec3), &modify_print_sec4, this);
        AnalsisSec3(&(bufrmsg.sec3));
    }
    else {
        process_bufrsec4(&(bufrmsgs[Row].sec4), &(bufrmsgs[Row].sec3), &modify_print_sec4, this);
        AnalsisSec3(&(bufrmsgs[Row].sec3));
    }
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //列自适应列宽
}

void BufAnalysisWin::appendRow(QList<QStandardItem*>& items) {
    model->appendRow(items);
}

void BufAnalysisWin::appendRow3(QList<QStandardItem*>& items) {
    model3->appendRow(items);
}

void BufAnalysisWin::cleartable() {
    /*清空两个控件*/
    model->removeRows(0, model->rowCount());
    ui->listWidget->clear();
}

void BufAnalysisWin::FreeMessage() {
    /*释放内容信息*/
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
    /*另存为功能, 直接调用库接口函数输出到txt文件中*/
    QString filename = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("另存为"), "", tr("txt(*.txt);;AllFile(*.*)"));
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
    /*设置窗口标题*/
    if(isFiles){
        this->setWindowTitle(QString::fromLocal8Bit(TITLESTR) + QString::fromLocal8Bit("-") + QString::fromLocal8Bit(MULTIPARTITE) + QString::fromLocal8Bit(": ") + FileName);
    }else{
        this->setWindowTitle(QString::fromLocal8Bit(TITLESTR) + QString::fromLocal8Bit("-") + QString::fromLocal8Bit(SINGE) + QString::fromLocal8Bit(": ") + FileName);
    }

}
