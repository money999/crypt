/***
 *自适应窗体大小搞的不好，以后再弄
 * 小文件多的时候会出现假死的情况，小文件200这样就会出现了
 * 可能是由于做主线程处理ui太复杂了？2.0是添加了textedit高亮光标会跟随移动的
 * 后期发现release版本不会出现上述说的假死现象
 * 问题解决了，写在博客里了
 * 2.1修复了bug，QEvent拿去delete后部分机子会报错，才发现上
 * 2.2新增可以一次多动多个文件
 * */



#include "cryptfile.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QObject>
#include <QPixmap>
#include <QWidget>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <qdebug.h>
#include <QByteArray>
#include <QObject>
#include <QRegExp>
#include <QValidator>
#include <QScrollBar>
#include <QFont>
#include <QMimeData>
#include <QTextBlock>
#include "crypt.h"

CryptFile::CryptFile(QWidget *parent)
    : QDialog(parent)
{
    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowMinimizeButtonHint;
    flags |=Qt::WindowCloseButtonHint;
    flags |=Qt::WindowMaximizeButtonHint;
    setWindowFlags(flags);

    setWindowTitle(tr("CrpytFile by fzj v2.2"));
    setWindowIcon(QIcon(":/image/lyd.png"));


    lbFileHint = new QLabel(tr("文件目录"));
    leFileAddress = new QLineEdit();
    pbFileOpen = new QPushButton(tr("导入"));
    pbStart = new QPushButton(tr("执行"));
    pbExplain = new QPushButton(tr("说明"));
    pbSummary = new QPushButton(tr("自定义"));
    lbImageDlam = new QLabel();
    lbFileSize = new QLabel(tr("---总共添加 0 个文件---"));
    teDisplay = new QTextEdit();
    lbSummary = new QLabel(tr("文件名：数字  后缀：lib"));
    lbCrypt = new QLabel(tr("输入密码"));
    leCrypt = new QLineEdit();


    leFileAddress->setReadOnly(true);
    lbImageDlam->setPixmap(QPixmap(":/image/dlam.png"));
    lbFileSize->setAlignment(Qt::AlignHCenter);
    teDisplay->setReadOnly(true);
    teDisplay->setWordWrapMode(QTextOption::NoWrap);
    teDisplay->setPlaceholderText(tr("将文件或文件夹拖入此处，可一次拖动多个文件或文件夹"));
    teDisplay->installEventFilter(this);
    teDisplay->setAcceptDrops(true);


    newFileName = tr("");
    newSuffix = tr(".lib");


    QHBoxLayout *main1=new QHBoxLayout();
    main1->setMargin(5);
    main1->setSpacing(5);
    main1->addWidget(lbFileHint);
    main1->addWidget(leFileAddress);
    main1->addWidget(pbFileOpen);

    QHBoxLayout *main2=new QHBoxLayout();
    main2->setMargin(5);
    main2->setSpacing(5);
    main2->addWidget(lbImageDlam);
    main2->addWidget(teDisplay);

    QHBoxLayout *main3=new QHBoxLayout();
    main3->addWidget(pbStart);
    main3->addWidget(pbExplain);

    QHBoxLayout *main4=new QHBoxLayout();
    main4->addWidget(lbSummary);
    main4->addWidget(pbSummary);

    QHBoxLayout *main5=new QHBoxLayout();
    main5->setMargin(5);
    main5->setSpacing(5);
    main5->addWidget(lbFileSize);

    QHBoxLayout *main6=new QHBoxLayout();
    main6->setMargin(5);
    main6->setSpacing(5);
    main6->addWidget(lbCrypt);
    main6->addWidget(leCrypt);


    QVBoxLayout *layout=new QVBoxLayout(this);
    layout->addLayout(main1);
    layout->addLayout(main6);
    layout->addLayout(main2);
    layout->addLayout(main5);
    layout->addLayout(main4);
    layout->addLayout(main3);

    connect(pbFileOpen,SIGNAL(clicked()),this,SLOT(setFileRoute()));
    connect(pbStart,SIGNAL(clicked()),this,SLOT(startCrypt()));
    connect(pbSummary,SIGNAL(clicked()),this,SLOT(runCustomm()));
    //connect(pbExplain,SIGNAL(clicked()),this,SLOT(runExplain()));

    /***
     * Qt5新语法，runexplian可以不用写slot里面了，槽函数可以是任意函数，
     * 这么写可以在编译的时候检查错误
     * 函数出现重载时要增加一个显式转换staticz_cast
     * */
    connect(pbExplain,&QPushButton::clicked,this, &CryptFile::runExplain);

    fileNum = 1;
}

CryptFile::~CryptFile()
{
}

void CryptFile::recursionDir(QString path, QString prefix)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDot|QDir::NoDotAndDotDot);
    QList<QFileInfo> fileInfo(dir.entryInfoList());
    for(int i=0;i<fileInfo.count();i++)
    {
        QString fPath = fileInfo.at(i).filePath();
        if(fileInfo.at(i).isFile())
        {
            fileRounts += fPath;
            QStringList t = fPath.split("/");
            fileNames += QString("%1、").arg(fileNum) + prefix + t.last();
            fileNames += "\n";
            fileNum++;
        }else if(fileInfo.at(i).isDir())
        {
            QStringList t = fPath.split("/");
            recursionDir(fileInfo.at(i).filePath(), prefix + t.last() + QDir::separator());
        }
    }
}

bool CryptFile::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == teDisplay) {
        if (event->type() == QEvent::DragEnter) {
            // [[2]]: 当拖放时鼠标进入label时, label接受拖放的动作
            QDragEnterEvent *dee = dynamic_cast<QDragEnterEvent *>(event);
            dee->acceptProposedAction();
            return true;
        } else if (event->type() == QEvent::Drop) {
            // [[3]]: 当放操作发生后, 取得拖放的数据
            QDropEvent *de = dynamic_cast<QDropEvent *>(event);
            QList<QUrl> urls = de->mimeData()->urls();
            if (urls.isEmpty()) { return true; }

            fileNames.clear();
            fileRounts.clear();
            fileNum = 1;

            foreach(QUrl url, urls){
                QString path = url.toLocalFile();
                QFileInfo pathInfo(path);
                if(pathInfo.isDir())
                {
                    QStringList t = path.split("/");
                    recursionDir(path, t.last() + QDir::separator());
                }else if(pathInfo.isFile())
                {
                    fileRounts += path;
                    fileNames += QString("%1、").arg(fileNum) + pathInfo.fileName();
                    fileNames += "\n";
                    fileNum++;
                }
            }
            QString path = urls.first().toLocalFile();
            QString t = path.left(path.lastIndexOf("/")+1);
            leFileAddress->setText(QDir::toNativeSeparators(t));
            teDisplay->setText(fileNames);
            lbFileSize->setText("---总共添加 "+QString::number(fileRounts.size(),10)+" 个文件---");

            QStringList tmp = fileNames.split("\n");
            int maxLength = 0;
            for(int i=0;i<tmp.count();i++)
            {
                QString tt = tmp.at(i);
                if(maxLength < tt.length())
                    maxLength = tt.length();
            }

            int screenwi = 466;
            if ((maxLength + 33) * 7 +187 > 466)
                screenwi = (maxLength + 33) * 7 +187;
            if (screenwi > 1500)
                screenwi = 1500;
            this->resize(screenwi,450);

            //delete de;//2.1修复此bug仅在部分机子上出现内存错误！
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void CryptFile::setFileRoute()
{
    fileNames.clear();
    fileRounts.clear();
    fileNum = 1;
    fileRounts += QFileDialog::getOpenFileNames(this,tr("选择文件"),"/","任意文件(*.*)");
    if(fileRounts.size() != 0){
        QStringList direction = fileRounts[0].split("/");
        direction.removeAt(direction.size()-1);
        leFileAddress->setText(direction.join("\\")+"\\");

        for(int i=0;i<fileRounts.size();i++){
            QStringList t = fileRounts[i].split("/");
            fileNames += QString("%1、").arg(fileNum) + t.at(t.size()-1);
            fileNum ++;
            fileNames += "\n";
        }
        teDisplay->setText(fileNames);
    }
    lbFileSize->setText("---总共添加 "+QString::number(fileRounts.size(),10)+" 个文件---");
}

void CryptFile::startCrypt()
{
    if(fileRounts.size() == 0){
        QMessageBox::warning(this,tr("警告"),tr("请先选择文件"),QMessageBox::Cancel);
    }else
    {
        if(leCrypt->text().length() == 0){
            QMessageBox::warning(this,tr("警告"),tr("请输入密码"),QMessageBox::Cancel);
        }else
        {
            pbExplain->setEnabled(false);
            pbFileOpen->setEnabled(false);
            pbStart->setEnabled(false);
            pbSummary->setEnabled(false);
            otStart=new OperaThread(fileRounts,leCrypt->text(), newFileName, newSuffix);
            connect(otStart,SIGNAL(update_m(QString, int, int)),this,SLOT(update_n(QString, int, int)), Qt::BlockingQueuedConnection);
            otStart->start();
        }
    }
}

void CryptFile::update_n(QString result, int point, int count)
{
    QStringList t = fileNames.split("\n");
    QString tt = t.at(point);
    if(count == -1)
    {
        tt += "  ->  "+result;
    }else
    {
        tt += "  ->  "+newFileName+QString::number(count, 10)+newSuffix+"   "+result;
    }
    t.replace(point,tt);
    fileNames.clear();
    fileNames = t.join("\n");

   //将光标移动到point行
    teDisplay->setText(fileNames);
    QTextBlock block = teDisplay->document()->findBlockByNumber(point);
    teDisplay->setTextCursor(QTextCursor(block));

    //高亮光标所在行
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(Qt::yellow);
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.cursor = teDisplay->textCursor();
    sel.cursor.clearSelection();
    extraSelections.append(sel);
    teDisplay->setExtraSelections(extraSelections);


    if(result != "decrypt success" && result != "encrypt success")
    {
        resultDisplay += tt;
        resultDisplay += '\n';
    }

    if(point == fileRounts.size()-1)
    {
        pbExplain->setEnabled(true);
        pbFileOpen->setEnabled(true);
        pbStart->setEnabled(true);
        pbSummary->setEnabled(true);
        fileRounts.clear();
        leFileAddress->clear();
        if(resultDisplay.length() > 0)
        {
            QMessageBox::warning(this,tr("警告"),resultDisplay,QMessageBox::Cancel);
        }
        else
        {
            QMessageBox::information(this,tr("提示"),tr("全部执行成功"),QMessageBox::Cancel);

        }

        resultDisplay.clear();
    }
}

void CryptFile::parameters_n(QString filename, QString suffix)
{
    if(filename == "")
    {
        filename = "数字";
    }else
    {
        newFileName = filename;
        filename += "+数字";

    }
    if(suffix == "")
    {
        suffix = ".fzj";
    }else
    {
        newSuffix = "."+suffix;
    }

    lbSummary->setText("文件名：" + filename + "  后缀：" + suffix);
}

void CryptFile::runCustomm()
{
    setCustomm = new Customm(this);
    connect(setCustomm,SIGNAL(parameters_m(QString,QString)),this,SLOT(parameters_n(QString,QString)));
    setCustomm->show();
}

void CryptFile::runExplain()
{
    setExplain = new Explain(this);
    setExplain->show();

}

Customm::Customm(QWidget *parent): QDialog(parent)
{
    lbFileName = new QLabel(tr("文件名:"));
    lbSuffix = new QLabel(tr("后缀名:"));
    leFileName = new QLineEdit();
    leSuffix = new QLineEdit();
    pbSure = new QPushButton(tr("确定"));
    pbCancel = new QPushButton(tr("取消"));
    lbFileNameHint = new QLabel(tr("文件名不能包含以下字符 \\ / * : ? \" < > |"));
    lbSuffixHint = new QLabel(tr("后缀名使用少于5个字符的数字及字母组合"));


    QRegExp regx("^[0-9a-zA-Z]{0,5}$");
    QValidator *validator = new QRegExpValidator(regx, leSuffix);
    leSuffix->setValidator(validator);

    QRegExp regx2("[^\\\\/*:?\"<>|]{0,100}");
    QValidator *validator2 = new QRegExpValidator(regx2, leFileName);
    leFileName->setValidator(validator2);


    QHBoxLayout *main1=new QHBoxLayout();
    main1->addWidget(lbFileName);
    main1->addWidget(leFileName);

    QHBoxLayout *main2=new QHBoxLayout();
    main2->addWidget(lbSuffix);
    main2->addWidget(leSuffix);

    QHBoxLayout *main3=new QHBoxLayout();
    main3->addWidget(pbSure);
    main3->addWidget(pbCancel);

    QHBoxLayout *main4=new QHBoxLayout();
    main4->addWidget(lbFileNameHint);

    QHBoxLayout *main5=new QHBoxLayout();
    main5->addWidget(lbSuffixHint);

    QVBoxLayout *layout=new QVBoxLayout(this);
    layout->addLayout(main1);
    layout->addLayout(main4);
    layout->addLayout(main2);
    layout->addLayout(main5);
    layout->addLayout(main3);

    connect(pbSure,SIGNAL(clicked()),this,SLOT(sure()));
    connect(pbCancel,SIGNAL(clicked()),this,SLOT(cancel()));
}

Customm::~Customm()
{

}

void Customm::sure()
{


    emit parameters_m(leFileName->text(), leSuffix->text());
    this->close();
}

void Customm::cancel()
{
    this->close();
}


OperaThread::OperaThread(QStringList cfileRounts, QString cpassword, QString cnewFileName, QString cnewSuffix)
{
    fileRounts = cfileRounts;
    password = cpassword;
    newFileName = cnewFileName;
    newSuffix = cnewSuffix;
}

void OperaThread::run()
{
    char *passwordch ;
    QByteArray baa = password.toLocal8Bit();
    passwordch = baa.data();


    for(int i=0, count=1; i<fileRounts.size(); i++)
    {
        QString t = fileRounts.at(i);
        t.replace(QString("/"), QString("\\"));

        char *fileAddress ;
        QByteArray ba = t.toLocal8Bit();
        fileAddress=ba.data();

        char *filenamech;
        QString name = newFileName + QString::number(count,10) + newSuffix;
        QByteArray filenamecharry = name.toLocal8Bit();
        filenamech = filenamecharry.data();

        //调用动态链接库里的函数
        char *resultch = XORCryptProof(fileAddress, passwordch, 50000, filenamech);


        QString result = QString(QLatin1String(resultch));

        if(result =="encrypt success")
        {
            emit update_m(result, i, count);
            count++;
        }else
        {
            emit update_m(result, i, -1);
        }
    }
}

Explain::Explain(QWidget *parent): QDialog(parent)
{
    QString ex = "1、软件不区分加密及未加密文件，当导入文件未被本软件加密过，则执行加密操作。当导\n入被本软件加密过的文件时则执行解密操作。软件无法对同一个文件连续加密两次\n\n2、在加密时默认强制修改文件名，默认为数字.lib，多个文件则递增。在自定义中可添加\n文件名前缀，如设置文件名为abc,后缀名为dat。则加密后的文件名为abc1.dat，abc2.\ndat。\n\n3、加密后若提示encrypt success but rename error，则可能由于您设置的文件名\n存在重名的情况，也可能该文件属性为隐藏或系统文件则更名失败\n\n4、解密的时候需要输入正确密码，若密码遗忘则——无解\n\n5、解密后会自动恢复原文件名，若提示decrypt success but rename error，请检\n查文件是否重名\n\n6、本软件遵循LGPL协议，但不作为任何商业用途，仅供学习和交流";
    lbImage = new QLabel();
    lbCopyRight = new QLabel(tr("Copyright@2014,fzj"));
    lbcontext = new QLabel(ex);

    lbImage->setPixmap(QPixmap(":/image/yxy.png"));
    lbImage->setAlignment(Qt::AlignHCenter);
    lbCopyRight->setAlignment(Qt::AlignHCenter);

    QFont ft;
    ft.setPointSize(20);
    lbCopyRight->setFont(ft);

    QHBoxLayout *main1=new QHBoxLayout();
    main1->addWidget(lbImage);

    QHBoxLayout *main2=new QHBoxLayout();
    main2->addWidget(lbCopyRight);

    QHBoxLayout *main3=new QHBoxLayout();
    main3->addWidget(lbcontext);

    QVBoxLayout *layout=new QVBoxLayout(this);
    layout->addLayout(main1);
    layout->addLayout(main2);
    layout->addLayout(main3);

}
Explain::~Explain()
{

}














