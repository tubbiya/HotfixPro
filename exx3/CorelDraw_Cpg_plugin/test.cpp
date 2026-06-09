#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    try {
        QString path = "olmayan_dosya.txt";
        QFile file(path);

        if (!file.open(QIODevice::ReadOnly)) {
            throw std::runtime_error(("Dosya açılamadı: " + path).toStdString());
        }

        // Normalde burada dosya okuma işlemi yapılır...
        qDebug() << "Dosya başarıyla açıldı!";
    }
    catch (const std::exception& e) {
        qCritical() << "Hata yakalandı:" << e.what();
        QMessageBox::critical(nullptr, "Hata", QString::fromStdString(e.what()));
    }

    return 0;
}
