#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QMessageBox>
#include <QImageReader>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // UI: Python’daki widget adlarını koru (örneğin):
    // - "btnOpen", "btnApply", "btnExport"
    // - "spinDotMM", "spinThreshold"
    // - "comboDaireRenkSec" (DaireRenkSec)
    connect(ui->btnOpen,   &QPushButton::clicked, this, &MainWindow::onOpenImage);
    connect(ui->btnApply,  &QPushButton::clicked, this, &MainWindow::onApplyDots);
    connect(ui->btnExport, &QPushButton::clicked, this, &MainWindow::onExportSvg);
    connect(ui->comboDaireRenkSec, &QComboBox::currentIndexChanged,
            this, &MainWindow::onColorMethodChanged);

    // Varsayılan parametreler (UI’den oku)
    dotDiameterMM = ui->spinDotMM->value();         // ör: 2.0
    threshold     = ui->spinThreshold->value();     // ör: 128

    // Combobox seçeneklerini doldur (Python’daki karşılıklarıyla)
    if (ui->comboDaireRenkSec->count() == 0) {
        ui->comboDaireRenkSec->addItem("Tek piksel",       (int)ColorMode::SinglePixel);
        ui->comboDaireRenkSec->addItem("3x3 ortalama",     (int)ColorMode::Mean3x3);
        ui->comboDaireRenkSec->addItem("5x5 ortalama",     (int)ColorMode::Mean5x5);
        ui->comboDaireRenkSec->addItem("Baskın renk (mod)",(int)ColorMode::Dominant);
    }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onOpenImage() {
    const QString file = QFileDialog::getOpenFileName(
        this, tr("Resim Aç"), QDir::homePath(),
        tr("Görüntüler (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)"));
    if (file.isEmpty()) return;

    QImageReader reader(file);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        QMessageBox::warning(this, tr("Hata"), tr("Görüntü okunamadı."));
        return;
    }

    image = img.convertToFormat(QImage::Format_RGBA8888);

    // DPI bilgisi varsa al
    if (image.dotsPerMeterX() > 0 && image.dotsPerMeterY() > 0) {
        dpiX = image.dotsPerMeterX() * 25.4 / 1000.0; // m->mm
        dpiY = image.dotsPerMeterY() * 25.4 / 1000.0;
    } else {
        dpiX = dpiY = 96.0; // Python’daki fallback değer
    }

    updatePixmap();
    resetView();
}

void MainWindow::updatePixmap() {
    scene->clear();
    imageItem = scene->addPixmap(QPixmap::fromImage(image));
    imageItem->setZValue(-1.0); // Daireler üstte kalsın
}

void MainWindow::resetView() {
    ui->graphicsView->resetTransform();
    scaleFactor = 1.0;
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::onApplyDots() {
    // UI’den güncel değerleri çek
    dotDiameterMM = ui->spinDotMM->value();
    threshold     = ui->spinThreshold->value();

    if (image.isNull()) {
        QMessageBox::information(this, tr("Uyarı"), tr("Önce bir resim açmalısın."));
        return;
    }

    // Python’daki apply_dots algoritmasını buraya birebir taşı:
    // 1) mm -> px dönüşümü
    const double dotPxX = mmToPx(dotDiameterMM, dpiX);
    const double dotPxY = mmToPx(dotDiameterMM, dpiY);
    const double radiusX = dotPxX / 2.0;
    const double radiusY = dotPxY / 2.0;

    // 2) Tarama/pattern (grid/hex vs.) – Python’daki mantığı KESİNLİKLE koru
    // 3) Eşik, keskinleştirme, hue/sat vs. (Python’da ne yaptıysan birebir)
    // 4) Renk seçimi: chooseDotColor(cx,cy) ile
    // 5) QPainterPath ile daire ekleme

    // --- ÖRNEK İSKELET (içi dolu DEĞİL, senin kodunu bekliyor) ---
    // for (double y = radiusY; y < image.height(); y += dotPxY) {
    //     for (double x = radiusX; x < image.width(); x += dotPxX) {
    //         // TODO: Python’daki koşullar (threshold, mask, path içinde mi vs.)
    //         QColor c = chooseDotColor(x, y);
    //         QPainterPath circle;
    //         circle.addEllipse(QPointF(x, y), radiusX, radiusY);
    //         auto *item = scene->addPath(circle, QPen(Qt::NoPen), QBrush(c));
    //         item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    //     }
    // }
    // -------------------------------------------------------------

    // Not: Üst üste binmeyi engellemek için Python’da ne yaptıysan
    // aynı kontrolü ekle (örneğin sahnedeki mevcut dairelerle mesafe kontrolü).
}

void MainWindow::onExportSvg() {
    if (!scene) return;
    const QString file = QFileDialog::getSaveFileName(
        this, tr("SVG Dışa Aktar"), QDir::homePath() + "/output.svg",
        tr("SVG (*.svg)"));
    if (file.isEmpty()) return;

    exportCurrentSceneToSvg(file);
    QMessageBox::information(this, tr("Tamam"), tr("SVG kaydedildi."));
}

void MainWindow::exportCurrentSceneToSvg(const QString &filePath) {
    // Python’daki export_svg fonksiyonundaki **ölçek/dpi** ayarlarını aynen uygula.
    // ÖNEMLİ: Resim ölçüsü ile SVG ölçüsü birebir olsun istiyorsan,
    // viewBox ve boyutu image pixel boyutları + dpi ilişkisine göre ayarla.

    QSvgGenerator gen;
    gen.setFileName(filePath);

    // Piksel boyutunu baz al
    const QSize sz(image.width(), image.height());
    gen.setSize(sz);
    gen.setViewBox(QRect(0, 0, sz.width(), sz.height()));
    gen.setTitle("Petek Export");
    gen.setDescription("Generated from scene");

    // dpi -> dotsPerMeter
    const double dpmX = dpiX / 25.4 * 1000.0;
    const double dpmY = dpiY / 25.4 * 1000.0;
    gen.setResolution(int((dpiX + dpiY) * 0.5)); // Qt’nin genel DPI alanı
    // Not: Qt’nin QSvgGenerator’da DPI davranışı sınırlı olabilir.
    // Piksel-bazlı boyutlandırma ile Corel’de mm karşılığı için ayrı ölçek tablosu tutuyorsan,
    // Python’daki ölçeği birebir uygula.

    QPainter p(&gen);
    ui->graphicsView->render(&p); // veya doğrudan scene->render(&p);
    p.end();
}

void MainWindow::onColorMethodChanged(int idx) {
    const auto mode = static_cast<ColorMode>(ui->comboDaireRenkSec->currentData().toInt());
    colorMode = mode;
}

double MainWindow::mmToPx(double mm, double dpi) const {
    // 1 inch = 25.4 mm
    return (mm / 25.4) * dpi;
}

double MainWindow::pxToMm(double px, double dpi) const {
    return (px / dpi) * 25.4;
}

QColor MainWindow::sampleColorAt(double x, double y) const {
    // Sınır kontrolü
    const int ix = std::clamp<int>(int(std::round(x)), 0, image.width()  - 1);
    const int iy = std::clamp<int>(int(std::round(y)), 0, image.height() - 1);
    return QColor::fromRgba(image.pixel(ix, iy));
}

QColor MainWindow::chooseDotColor(double cx, double cy) const {
    // Python’daki DaireRenkSec mantığını aynen taşı:
    // SinglePixel: tek piksel
    // Mean3x3 / Mean5x5: kernel ortalaması
    // Dominant: küçük pencerede en sık renk
    switch (colorMode) {
        case ColorMode::SinglePixel:
            return sampleColorAt(cx, cy);
        case ColorMode::Mean3x3: {
            // TODO: 3x3 ortalama (Python’daki kodunu taşı)
            return sampleColorAt(cx, cy);
        }
        case ColorMode::Mean5x5: {
            // TODO: 5x5 ortalama
            return sampleColorAt(cx, cy);
        }
        case ColorMode::Dominant: {
            // TODO: mod renk
            return sampleColorAt(cx, cy);
        }
    }
    return sampleColorAt(cx, cy);
}

// -------- Pan & Zoom --------
void MainWindow::wheelEvent(QWheelEvent *e) {
    constexpr double zoomStep = 1.15;
    if (e->angleDelta().y() > 0) {
        scaleFactor *= zoomStep;
        ui->graphicsView->scale(zoomStep, zoomStep);
    } else {
        scaleFactor /= zoomStep;
        ui->graphicsView->scale(1.0/zoomStep, 1.0/zoomStep);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::MiddleButton) {
        panning = true;
        lastPanPos = e->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QMainWindow::mousePressEvent(e);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e) {
    if (panning) {
        QPointF delta = ui->graphicsView->mapToScene(lastPanPos) - ui->graphicsView->mapToScene(e->pos());
        ui->graphicsView->translate(delta.x(), delta.y());
        lastPanPos = e->pos();
    }
    QMainWindow::mouseMoveEvent(e);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::MiddleButton) {
        panning = false;
        setCursor(Qt::ArrowCursor);
    }
    QMainWindow::mouseReleaseEvent(e);
}
