# =============================
# CMakeLists.txt
# =============================
cmake_minimum_required(VERSION 3.24)
project(PetekCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Gui)

add_executable(PetekCpp
    src/main.cpp
    src/MainWindow.cpp
    src/ImageProcessor.cpp
    src/SvgExporter.cpp
    src/Utils.cpp
    
    include/MainWindow.h
    include/ImageProcessor.h
    include/SvgExporter.h
    include/Utils.h
)

target_include_directories(PetekCpp PRIVATE include)

target_link_libraries(PetekCpp PRIVATE Qt6::Widgets Qt6::Svg Qt6::Gui)

# =============================
# src/main.cpp
# =============================
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("PetekCpp");
    QApplication app(argc, argv);

    MainWindow w;
    w.show();
    return app.exec();
}

# =============================
# include/MainWindow.h
# =============================
#pragma once
#include <QMainWindow>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <memory>

class ImageProcessor;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onOpenImage();
    void onApplyDots();
    void onExportSvg();

private:
    void buildUi();
    void connectSignals();
    void renderImageAndDots();

    QGraphicsView *view_ {nullptr};
    QGraphicsScene *scene_ {nullptr};

    std::unique_ptr<ImageProcessor> proc_;
};

# =============================
# src/MainWindow.cpp
# =============================
#include "MainWindow.h"
#include "ImageProcessor.h"
#include "SvgExporter.h"
#include <QToolBar>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), proc_(std::make_unique<ImageProcessor>()) {
    buildUi();
    connectSignals();
}

void MainWindow::buildUi() {
    scene_ = new QGraphicsScene(this);
    view_ = new QGraphicsView(scene_, this);
    setCentralWidget(view_);

    auto *tb = addToolBar("Araçlar");
    auto *actOpen = tb->addAction("Resim Aç");
    auto *actApply = tb->addAction("Uygula (Dots)");
    auto *actExport = tb->addAction("SVG Dışa Aktar");

    actOpen->setObjectName("actionOpen");
    actApply->setObjectName("actionApply");
    actExport->setObjectName("actionExport");

    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpenImage);
    connect(actApply, &QAction::triggered, this, &MainWindow::onApplyDots);
    connect(actExport, &QAction::triggered, this, &MainWindow::onExportSvg);
}

void MainWindow::connectSignals() {}

void MainWindow::onOpenImage() {
    const QString path = QFileDialog::getOpenFileName(this, "Resim Seç", QString(),
                                                      "Görseller (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)");
    if (path.isEmpty()) return;
    if (!proc_->loadImage(path)) {
        QMessageBox::warning(this, "Hata", "Resim yüklenemedi.");
        return;
    }
    renderImageAndDots();
}

void MainWindow::onApplyDots() {
    // Örnek varsayılanlar — Python tarafında ne yaptıysanız bunlara bağlayın
    proc_->setDpiOverride(0); // 0 = otomatik DPI tespiti
    proc_->setStoneDiameterMM(2.0); // 2 mm
    proc_->setSpacingMM(0.0);       // taşlar arası ekstra boşluk
    proc_->setLayoutType(ImageProcessor::Layout::Hex); // Grid/Hex/Serbest

    proc_->applyDots();
    renderImageAndDots();
}

void MainWindow::onExportSvg() {
    if (!proc_->hasImage()) {
        QMessageBox::information(this, "Bilgi", "Önce bir resim yükleyin.");
        return;
    }
    const QString out = QFileDialog::getSaveFileName(this, "SVG Kaydet", "petek.svg", "SVG (*.svg)");
    if (out.isEmpty()) return;

    SvgExporter exporter;
    // mm -> piksel dönüşümü içeride DPI ile yapılır
    const bool ok = exporter.exportSvg(out, *proc_);
    if (!ok) QMessageBox::warning(this, "Hata", "SVG dışa aktarım başarısız oldu.");
}

void MainWindow::renderImageAndDots() {
    scene_->clear();

    if (proc_->hasImage()) {
        QPixmap pm = QPixmap::fromImage(proc_->image());
        scene_->addPixmap(pm);

        // Daireleri QGraphicsEllipseItem olarak çiz
        const auto &dots = proc_->dots();
        const double r_px = proc_->stoneRadiusPx();
        for (const QPointF &p : dots) {
            auto *el = scene_->addEllipse(QRectF(p.x()-r_px, p.y()-r_px, 2*r_px, 2*r_px), QPen(Qt::NoPen), QBrush(Qt::gray));
            el->setOpacity(0.7);
        }
        scene_->setSceneRect(pm.rect());
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    }
}

# =============================
# include/ImageProcessor.h
# =============================
#pragma once
#include <QImage>
#include <QPointF>
#include <vector>

class ImageProcessor {
public:
    enum class Layout { Grid, Hex, Free };

    bool loadImage(const QString &path);
    bool hasImage() const { return !img_.isNull(); }
    const QImage &image() const { return img_; }

    // Parametreler
    void setStoneDiameterMM(double mm) { stone_d_mm_ = mm; }
    void setSpacingMM(double mm) { spacing_mm_ = mm; }
    void setLayoutType(Layout t) { layout_ = t; }
    void setDpiOverride(int dpi) { dpi_override_ = dpi; } // 0 => otomatik

    // İşlemler
    void applyDots();

    // Çıktılar
    const std::vector<QPointF>& dots() const { return dots_; }
    double stoneRadiusPx() const { return stone_r_px_; }

    // Dönüşümler
    int effectiveDpi() const; // EXIF, PNG pHYs, vs. üzerinden bulmaya çalışır; yoksa 96
    double pxPerMM() const;   // dpi / 25.4

private:
    void computeStoneRadiusPx();
    void layoutGrid();
    void layoutHex();

private:
    QImage img_;
    std::vector<QPointF> dots_;

    // Parametreler
    double stone_d_mm_ {2.0};
    double spacing_mm_ {0.0};
    Layout layout_ {Layout::Hex};
    int dpi_override_ {0};

    // Türetilen
    double stone_r_px_ {1.0};
};

# =============================
# src/ImageProcessor.cpp
# =============================
#include "ImageProcessor.h"
#include "Utils.h"
#include <QtGui/QImageReader>

bool ImageProcessor::loadImage(const QString &path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    img_ = reader.read();
    dots_.clear();
    if (img_.isNull()) return false;
    computeStoneRadiusPx();
    return true;
}

int ImageProcessor::effectiveDpi() const {
    if (dpi_override_ > 0) return dpi_override_;
    // QImage DPI -> dots per meter veriyor; bunu DPI'a çevir
    const double xppm = img_.dotsPerMeterX();
    if (xppm > 0) return static_cast<int>(xppm * 25.4 / 1000.0 + 0.5);
    // Yedek: 96 DPI (Windows standart)
    return 96;
}

double ImageProcessor::pxPerMM() const {
    return static_cast<double>(effectiveDpi()) / 25.4;
}

void ImageProcessor::computeStoneRadiusPx() {
    const double ppx = pxPerMM();
    stone_r_px_ = (stone_d_mm_ * ppx) * 0.5; // yarıçap
}

void ImageProcessor::applyDots() {
    dots_.clear();
    computeStoneRadiusPx();

    switch (layout_) {
        case Layout::Grid: layoutGrid(); break;
        case Layout::Hex:  layoutHex();  break;
        case Layout::Free: /* TODO: Serbest yerleşim */ break;
    }
}

void ImageProcessor::layoutGrid() {
    const double step_px = 2.0 * stone_r_px_ + spacing_mm_ * pxPerMM();
    for (double y = stone_r_px_; y < img_.height(); y += step_px) {
        for (double x = stone_r_px_; x < img_.width(); x += step_px) {
            dots_.emplace_back(x, y);
        }
    }
}

void ImageProcessor::layoutHex() {
    const double ppx = pxPerMM();
    const double d_px = 2.0 * stone_r_px_;
    const double step_x = d_px + spacing_mm_ * ppx;
    const double step_y = (d_px + spacing_mm_ * ppx) * 0.866025403784; // sqrt(3)/2

    bool odd = false;
    for (double y = stone_r_px_; y < img_.height(); y += step_y) {
        double x0 = stone_r_px_ + (odd ? step_x * 0.5 : 0.0);
        for (double x = x0; x < img_.width(); x += step_x) {
            dots_.emplace_back(x, y);
        }
        odd = !odd;
    }
}

# =============================
# include/SvgExporter.h
# =============================
#pragma once
#include <QString>
#include <QSizeF>
class ImageProcessor;

class SvgExporter {
public:
    // ImageProcessor'dan resim boyutu (px) ve taş listesi alınır
    bool exportSvg(const QString &filePath, const ImageProcessor &proc) const;
};

# =============================
# src/SvgExporter.cpp
# =============================
#include "SvgExporter.h"
#include "ImageProcessor.h"
#include <QSvgGenerator>
#include <QPainter>
#include <QFile>
#include <QTextStream>

// Basit, sağlam yol: QSvgGenerator ile px tabanlı çıktı ve viewBox eşlemesi.
// Fiziksel mm doğruluğu için: width/height'ı mm cinsinden doğrudan dosyaya yazmak isterseniz
// QSvgGenerator yerine kendi XML yazıcınızı kullanın. İlk sürüm için generator yeterlidir.

bool SvgExporter::exportSvg(const QString &filePath, const ImageProcessor &proc) const {
    if (!QFile::open(QFile(filePath).fileName().toUtf8().constData(), QIODevice::WriteOnly)) {
        // QFile ile ayrı açalım ki boş dosya aşaması oluşmasın
    }

    QSvgGenerator gen;
    gen.setFileName(filePath);

    const auto &img = proc.image();
    const QSize sizePx = img.size();
    gen.setSize(sizePx);
    gen.setViewBox(QRect(0,0,sizePx.width(), sizePx.height()));
    gen.setTitle("Petek SVG");
    gen.setDescription("Taş yerleşimi SVG çıktısı");

    QPainter p;
    if (!p.begin(&gen)) return false;

    // Arka plana görüntüyü yerleştirmek isterseniz (isteğe bağlı):
    // p.drawImage(QPoint(0,0), img);

    const double r = proc.stoneRadiusPx();
    for (const QPointF &pt : proc.dots()) {
        p.setBrush(Qt::black);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(pt.x(), pt.y()), r, r);
    }
    p.end();

    // NOT: CorelDRAW'da mm birebir eşleşmesi isteniyorsa, aşağıdaki yönteme geçebilirsiniz:
    //  - pxPerMM = dpi/25.4
    //  - width_mm  = width_px  / pxPerMM
    //  - height_mm = height_px / pxPerMM
    //  - radius_mm = radius_px / pxPerMM
    //  - width="{width_mm}mm" height="{height_mm}mm" viewBox="0 0 {width_mm} {height_mm}"
    //  - circle cx, cy, r değerlerini mm ile yazın.
    // İlerleyen adımda istenirse bu XML tabanlı yazımı ekleriz.

    return true;
}

# =============================
# include/Utils.h
# =============================
#pragma once
#include <QtGlobal>

namespace Utils {
    template<typename T> inline T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }
}

# =============================
# src/Utils.cpp
# =============================
#include "Utils.h"

// Şimdilik boş; gerektiğinde yardımcı fonksiyonlar eklenecek

# =============================
# README (kısa notlar)
# =============================
// 1) Kurulum
// - Qt 6.6+ (Widgets, Svg) kurulu olmalı.
// - Windows + MSVC için: "Qt 6.x for MSVC" kitini kurun.
// - CMake + Ninja (opsiyonel) veya Visual Studio 2022 ile açın.
//
// 2) Derleme
// cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.6.3/msvc2019_64"
// cmake --build build
//
// 3) Visual Studio ile açmak
// - VS'de: File > Open > CMake...
// - Veya "cmake -G \"Visual Studio 17 2022\"" ile proje üretin ve .sln açın.
//
// 4) Python fonksiyon isim eşlemesi
//   PyQt6           ->    C++/Qt6
//   load_image()    ->    ImageProcessor::loadImage
//   apply_dots()    ->    ImageProcessor::applyDots (Layout: Grid/Hex)
//   export_svg()    ->    SvgExporter::exportSvg
//   px/mm-DPI       ->    ImageProcessor::pxPerMM / effectiveDpi
//
// 5) Sonraki adımlar (TODO)
// - Renk seçimi/ortalama (QImage piksel okuma ile): taş rengine karar verip SVG'de fill olarak yaz.
// - Serbest yerleşim (Free) ve çakışma önleme: yeni daire varolanlara değerse kaydırma.
// - SVG'yi mm bazında yazan manuel XML exporter (Corel ile %100 fiziksel eşleşme için).
// - UI: Taş çapı (mm), boşluk (mm), Layout combobox, renk metodu combobox.
