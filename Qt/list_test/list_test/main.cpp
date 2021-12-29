#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::list<int*> items;
    for (size_t i = 0; i < 1000; i++)
        items.push_back(new int(i+1));

    int i = 0;
    for (std::list<int*>::const_iterator it = items.cbegin(); it != items.cend(); ++it)
        qDebug("%d: src_size=%d", i++, (int)items.size());

    return a.exec();
}
