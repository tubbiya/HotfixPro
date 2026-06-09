#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QImage>
#include <QVector>
#include <QListWidgetItem>
#include <QColor>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // PetekA.ui’den gelir
QT_END_NAMESPACE

struct Dot {
    float x, y, r;
    int paletteIndex;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void loadImage();
    void applyDots();
    void exportSvg();
    void onColorItemClicked(QListWidgetItem *item);
    void mergeColorsFromSlider();

private:
    // ==== UI & Scene ====
    Ui::MainWindow *ui;
    QGraphicsScene *scene_;
    QGraphicsPixmapItem *originalItem_{nullptr};
    QVector<QGraphicsPathItem*> dotItems_;
    QVector<Dot> dots_;

    // ==== Image & Integrals ====
    QImage imageRGBA_;
    int W_{0}, H_{0};
    // (H+1) x (W+1) integral tabloları (taşma için 64-bit)
    QVector<quint64> Ir_, Ig_, Ib_, Ia_;

    // ==== Palette ====
    QVector<QColor> PALETTE_, ORIGINAL_PALETTE_;
    static int colorDistance(const QColor &a, const QColor &b);
    static void mergeCloseColors(QVector<QColor> &cols, int tol);
    void updateColorList();
    void updateDotItemBrushes();

    // ==== Helpers ====
    void buildIntegrals();
    inline int idxI(int x, int y) const { return y*(W_+1) + x; }
    quint64 rectSum(const QVector<quint64>& I, int x0, int y0, int x1, int y1) const;
    QColor boxMeanRGB(int x0, int y0, int x1, int y1) const;

    void autoPalette(int count);
    void makeGrid(float dot_px, float spacing_px, bool hex,
                  QVector<float> &Xs, QVector<float> &Ys) const;
    int nearestPaletteIndex(const QColor &c) const;

    // ==== UI naming (PetekA.ui’deki objectName’ler) ====
    // graphicsView : QGraphicsView
    // DosyaAc      : QPushButton
    // Uygula       : QPushButton
    // Export       : QPushButton
    // renkLmt      : QListWidget
    // RenkBirTol   : QSlider
    // renkL        : QSpinBox (renk sayısı)
    // Olcu         : QDoubleSpinBox (mm)
    // Mesf         : QDoubleSpinBox (mm)
    // Petek        : QCheckBox (hex grid)
    // Opak         : QCheckBox (alfa>0 filtre)
    // DaireRenkSec : QComboBox (renklendirme yöntemi - basitçe ortalama uygulanır)
};
