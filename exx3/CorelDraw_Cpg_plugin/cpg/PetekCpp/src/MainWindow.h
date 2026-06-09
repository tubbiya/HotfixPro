#pragma once
#include <QMainWindow>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QSvgGenerator>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QComboBox>
#include <QSpinBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Python’daki akışa paralel slotlar
    void onOpenImage();       // load_image(...)
    void onApplyDots();       // apply_dots(...)
    void onExportSvg();       // export_svg(...)
    void onColorMethodChanged(int idx); // DaireRenkSec combobox

private:
    // Yardımcılar (Python’daki fonksiyonlar ile aynı isim/rol)
    void resetView();
    void updatePixmap();
    double mmToPx(double mm, double dpi) const; // mm->px dönüştürme
    double pxToMm(double px, double dpi) const; // px->mm dönüştürme

    // Pan/Zoom
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    // Renk seçimi/ortalama vs. (combobox ile seçilecek)
    QColor sampleColorAt(double x, double y) const;
    QColor chooseDotColor(double cx, double cy) const; // combobox yöntemine göre

    // SVG üretimi
    void exportCurrentSceneToSvg(const QString &filePath);

private:
    Ui::MainWindow *ui;

    // Sahne/öğeler
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *imageItem = nullptr;

    // Görüntü ve metrikler
    QImage image;
    double dpiX = 96.0; // Python’daki mantığa göre güncelle
    double dpiY = 96.0;

    // Zoom/Pan durumu
    double scaleFactor = 1.0;
    bool panning = false;
    QPoint lastPanPos;

    // Parametreler (Python kodundaki adları koru)
    double dotDiameterMM = 2.0; // örnek: UI’den okunur
    double threshold = 128.0;   // örnek: UI’den okunur

    // Renk yöntemi (Python’daki DaireRenkSec karşılığı)
    enum class ColorMode { SinglePixel, Mean3x3, Mean5x5, Dominant };
    ColorMode colorMode = ColorMode::SinglePixel;
};
