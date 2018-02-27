#include "cryptfile.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
   // QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale()); 5.0取消了这两个
   // QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    CryptFile w;
    w.show();

    return a.exec();
}
