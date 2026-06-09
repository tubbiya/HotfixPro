#include "mainwindow.h"
#include "ui_PetekA.h" // CMAKE_AUTOUIC sayesinde üretilir
#include <QtWidgets>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene_(new QGraphicsScene(this)) {
    ui->setupUi(this);

    ui->graphicsView->setScene(scene_);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform, true);
    ui->graphicsView->viewport()->installEventFilter(this);

    connect(ui->DosyaAc, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(ui->Uygula,  &QPushButton::clicked, this, &MainWindow::applyDots);
    connect(ui->Export,  &QPushButton::clicked, this, &MainWindow::exportSvg);
    connect(ui->renkLmt, &QListWidget::itemClicked, this, &MainWindow::onColorItemClicked);
    connect(ui->RenkBirTol, &QSlider::sliderReleased, this, &MainWindow::mergeColorsFromSlider);

    // Başlangıç ayarları
    ui->renkL->setMinimum(1);
    ui->Olcu->setMinimum(0.1);
    ui->Mesf->setMinimum(0.0);
    ui->renkL->setValue(15);
    ui->Olcu->setValue(25.0);
    statusBar()->showMessage("Hazır");

    ui->DaireRenkSec->clear();
    ui->DaireRenkSec->addItems({
        "Tek Piksel", "Ortalama Renk", "Köşe/Nokta Ortalaması", "En Çok Renk",
        "Medyan Renk", "Merkez Ağırlıklı Ort.", "Halka Örnekleme", "Hue (Ton) Baskınlığı"
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->graphicsView->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (originalItem_) originalItem_->setVisible(true);
            for (auto *it : dotItems_) if (it) it->setVisible(false);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            if (originalItem_) originalItem_->setVisible(false);
            for (auto *it : dotItems_) if (it) it->setVisible(true);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    const double z = (event->angleDelta().y() > 0) ? 1.25 : 1.0/1.25;
    ui->graphicsView->scale(z, z);
}

void MainWindow::loadImage() {
    const QString file = QFileDialog::getOpenFileName(this, "Resim Aç", QString(), "Images (*.png *.jpg *.bmp)");
    if (file.isEmpty()) return;

    QImage img(file);
    if (img.isNull()) {
        statusBar()->showMessage("Resim açılamadı.");
        return;
    }
    imageRGBA_ = img.convertToFormat(QImage::Format_RGBA8888);
    W_ = imageRGBA_.width();
    H_ = imageRGBA_.height();

    scene_->clear();
    originalItem_ = scene_->addPixmap(QPixmap::fromImage(imageRGBA_));
    originalItem_->setVisible(true);
    dotItems_.clear();
    dots_.clear();

    buildIntegrals();
    PALETTE_.clear();
    ORIGINAL_PALETTE_.clear();
    ui->renkLmt->clear();

    ui->graphicsView->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    statusBar()->showMessage(QString("%1 yüklendi").arg(file));
}

void MainWindow::buildIntegrals() {
    Ir_.fill(0); Ig_.fill(0); Ib_.fill(0); Ia_.fill(0);
    Ir_.resize((W_+1)*(H_+1));
    Ig_.resize((W_+1)*(H_+1));
    Ib_.resize((W_+1)*(H_+1));
    Ia_.resize((W_+1)*(H_+1));

    auto at = [&](const QVector<quint64> &I, int x, int y)->quint64 {
        return I[idxI(x,y)];
    };
    for (int y=1; y<=H_; ++y) {
        const uchar *row = imageRGBA_.constScanLine(y-1);
        quint64 sr=0, sg=0, sb=0, sa=0;
        for (int x=1; x<=W_; ++x) {
            const uchar *p = row + (x-1)*4; // RGBA8888
            const quint64 r = p[0], g = p[1], b = p[2], a = p[3];
            sr += r; sg += g; sb += b; sa += a;
            Ir_[idxI(x,y)] = Ir_[idxI(x,y-1)] + sr;
            Ig_[idxI(x,y)] = Ig_[idxI(x,y-1)] + sg;
            Ib_[idxI(x,y)] = Ib_[idxI(x,y-1)] + sb;
            Ia_[idxI(x,y)] = Ia_[idxI(x,y-1)] + sa;
        }
    }
}

quint64 MainWindow::rectSum(const QVector<quint64>& I, int x0, int y0, int x1, int y1) const {
    // x,y inclusive; integral tabloda 1 pad var
    if (x0 > x1) std::swap(x0,x1);
    if (y0 > y1) std::swap(y0,y1);
    x0 = std::clamp(x0, 0, W_-1);
    x1 = std::clamp(x1, 0, W_-1);
    y0 = std::clamp(y0, 0, H_-1);
    y1 = std::clamp(y1, 0, H_-1);
    // integraller 1 padli olduğu için +1
    return I[idxI(x1+1,y1+1)] - I[idxI(x0,y1+1)] - I[idxI(x1+1,y0)] + I[idxI(x0,y0)];
}

QColor MainWindow::boxMeanRGB(int x0, int y0, int x1, int y1) const {
    const quint64 area = quint64(std::max(1, (x1-x0+1)*(y1-y0+1)));
    const quint64 sr = rectSum(Ir_, x0,y0,x1,y1);
    const quint64 sg = rectSum(Ig_, x0,y0,x1,y1);
    const quint64 sb = rectSum(Ib_, x0,y0,x1,y1);
    return QColor(int(sr/area), int(sg/area), int(sb/area));
}

int MainWindow::colorDistance(const QColor &a, const QColor &b) {
    int dr=a.red()-b.red(), dg=a.green()-b.green(), db=a.blue()-b.blue();
    return int(std::lround(std::sqrt(double(dr*dr+dg*dg+db*db))));
}

void MainWindow::mergeCloseColors(QVector<QColor> &cols, int tol) {
    QVector<QColor> out;
    for (const QColor &c : cols) {
        bool merged=false;
        for (QColor &g : out) {
            if (colorDistance(c, g) < tol) {
                g.setRed((g.red()+c.red())/2);
                g.setGreen((g.green()+c.green())/2);
                g.setBlue((g.blue()+c.blue())/2);
                merged=true; break;
            }
        }
        if (!merged) out.push_back(c);
    }
    cols = out;
}

void MainWindow::autoPalette(int count) {
    if (W_<=0 || H_<=0) { PALETTE_.clear(); return; }
    const int stepY = std::max(1, H_/50);
    const int stepX = std::max(1, W_/50);
    QVector<QColor> sample;
    sample.reserve((H_/stepY+1)*(W_/stepX+1));
    for (int y=0; y<H_; y+=stepY) {
        const uchar *row = imageRGBA_.constScanLine(y);
        for (int x=0; x<W_; x+=stepX) {
            const uchar *p = row + x*4;
            sample.push_back(QColor(p[0],p[1],p[2]));
        }
    }
    if (sample.isEmpty()) sample.push_back(QColor(0,0,0));
    mergeCloseColors(sample, 35);
    PALETTE_.clear();
    for (int i=0; i<std::min(count, sample.size()); ++i) PALETTE_.push_back(sample[i]);
    if (PALETTE_.isEmpty()) PALETTE_.push_back(QColor(0,0,0));
    ORIGINAL_PALETTE_ = PALETTE_;
    updateColorList();
}

void MainWindow::makeGrid(float dot_px, float spacing_px, bool hex,
                          QVector<float> &Xs, QVector<float> &Ys) const {
    const float stepX = std::max(1.0f, dot_px + spacing_px);
    const float stepY = std::max(1.0f, (dot_px + spacing_px) * (hex ? 0.866f : 1.0f));
    const int cols = int(W_ / stepX);
    const int rows = int(H_ / stepY);
    if (cols < 1 || rows < 1) { Xs.clear(); Ys.clear(); return; }

    Xs.reserve(cols*rows);
    Ys.reserve(cols*rows);

    if (hex) {
        for (int r=0; r<rows; ++r) {
            const float y = r * stepY;
            const float offset = (r % 2 == 0) ? 0.0f : (stepX / 2.0f);
            for (int c=0; c<cols; ++c) {
                const float x = offset + c*stepX;
                Xs.push_back(x);
                Ys.push_back(y);
            }
        }
    } else {
        for (int r=0; r<rows; ++r) {
            const float y = r * stepY;
            for (int c=0; c<cols; ++c) {
                const float x = c * stepX;
                Xs.push_back(x);
                Ys.push_back(y);
            }
        }
    }
}

int MainWindow::nearestPaletteIndex(const QColor &c) const {
    int best = 0;
    int bestd = std::numeric_limits<int>::max();
    for (int i=0;i<PALETTE_.size();++i) {
        int d = colorDistance(c, PALETTE_[i]);
        if (d < bestd) { bestd = d; best = i; }
    }
    return best;
}

void MainWindow::applyDots() {
    if (imageRGBA_.isNull()) {
        QMessageBox::warning(this, "Uyarı", "Önce bir resim aç.");
        return;
    }
    if (originalItem_) originalItem_->setVisible(false);
    for (auto *it : dotItems_) if (it) scene_->removeItem(it);
    dotItems_.clear(); dots_.clear();

    const double dot_mm = std::max(1.0, ui->Olcu->value());
    const double spacing_mm = std::max(0.0, ui->Mesf->value());
    const bool hex = ui->Petek->isChecked();
    const int renkSay = std::max(1, ui->renkL->value());
    const bool opakOnly = ui->Opak->isChecked();

    // DPI: Qt’ta metre başına nokta -> dpi = dpm*0.0254
    double dpi_x = imageRGBA_.dotsPerMeterX() * 0.0254;
    if (dpi_x <= 1.0) dpi_x = 300.0; // varsayılan
    const float dot_px = float(dot_mm * dpi_x / 25.4);
    const float spacing_px = float(spacing_mm * dpi_x / 25.4);

    // Palet
    autoPalette(renkSay);

    // Grid
    QVector<float> Xs, Ys;
    makeGrid(dot_px, spacing_px, hex, Xs, Ys);
    if (Xs.isEmpty()) {
        statusBar()->showMessage("Yoğunluk 0: grid boş.");
        return;
    }

    // Örnekleme & atama
    const float r = dot_px/2.0f;
    QVector<QPainterPath> paths(PALETTE_.size());
    int kept = 0;

    for (int i=0; i<Xs.size(); ++i) {
        const float x = Xs[i];
        const float y = Ys[i];

        // Alfa filtresi için merkez piksel
        bool keep = true;
        if (opakOnly) {
            int cx = std::clamp(int(std::lround(x + r)), 0, W_-1);
            int cy = std::clamp(int(std::lround(y + r)), 0, H_-1);
            const uchar *p = imageRGBA_.constScanLine(cy) + cx*4;
            if (p[3] == 0) keep = false; // alpha == 0
        }
        if (!keep) continue;

        // Ortalama renk (kutu ortalaması)
        int x0 = int(std::floor(x));
        int y0 = int(std::floor(y));
        int x1 = int(std::floor(x + 2*r - 1e-3));
        int y1 = int(std::floor(y + 2*r - 1e-3));
        QColor mean = boxMeanRGB(x0,y0,x1,y1);
        int pi = nearestPaletteIndex(mean);

        paths[pi].addEllipse(QRectF(x, y, dot_px, dot_px));
        dots_.push_back({x,y,r,pi});
        ++kept;
    }

    // Çizim
    for (int i=0;i<paths.size();++i) {
        if (paths[i].isEmpty()) continue;
        auto *it = new QGraphicsPathItem(paths[i]);
        it->setBrush(QBrush(PALETTE_[i]));
        it->setPen(Qt::NoPen);
        scene_->addItem(it);
        dotItems_.push_back(it);
    }
    statusBar()->showMessage(QString("Daireler uygulandı: %1 nokta.").arg(kept));
}

void MainWindow::exportSvg() {
    if (dots_.isEmpty() || imageRGBA_.isNull()) {
        statusBar()->showMessage("Dışa aktarılacak desen yok.");
        return;
    }
    QString file = QFileDialog::getSaveFileName(this, "SVG Olarak Kaydet", QString(), "SVG Files (*.svg)");
    if (file.isEmpty()) return;
    if (!file.toLower().endsWith(".svg")) file += ".svg";

    double dpi_x = imageRGBA_.dotsPerMeterX() * 0.0254;
    if (dpi_x <= 1.0) dpi_x = 300.0;
    const int w_px = W_, h_px = H_;
    const double w_mm = w_px * 25.4 / dpi_x;
    const double h_mm = h_px * 25.4 / dpi_x;

    QStringList svg;
    svg << R"(<?xml version="1.0" encoding="UTF-8"?>)"
        << R"(<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.0//EN" "http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">)"
        << QString(R"(<svg xmlns="http://www.w3.org/2000/svg" width="%1mm" height="%2mm" version="1.0" )").arg(w_mm,0,'f',6).arg(h_mm,0,'f',6)
           + QString(R"(style="shape-rendering:geometricPrecision; text-rendering:geometricPrecision; image-rendering:optimizeQuality; fill-rule:evenodd; clip-rule:evenodd" viewBox="0 0 %1 %2">)").arg(w_px).arg(h_px)
        << " <defs>"
        << "  <style type=\"text/css\"><![CDATA[";

    // renk sınıfları
    QHash<QString, QString> colorClass;
    QStringList classLines;
    int classCount=0;
    for (const QColor &c : PALETTE_) {
        const QString hexc = QString("#%1%2%3")
            .arg(c.red(),2,16,QLatin1Char('0'))
            .arg(c.green(),2,16,QLatin1Char('0'))
            .arg(c.blue(),2,16,QLatin1Char('0')).toUpper();
        if (!colorClass.contains(hexc)) {
            const QString name = QString("fil%1").arg(classCount++);
            colorClass.insert(hexc, name);
            classLines << QString("   .%1 {fill:%2}").arg(name, hexc);
        }
    }
    svg << classLines;
    svg << "  ]]></style>";
    svg << " </defs>";
    svg << " <g id=\"Layer_1\">";

    for (const Dot &d : dots_) {
        const QColor &c = PALETTE_.at(d.paletteIndex);
        const QString hexc = QString("#%1%2%3")
            .arg(c.red(),2,16,QLatin1Char('0'))
            .arg(c.green(),2,16,QLatin1Char('0'))
            .arg(c.blue(),2,16,QLatin1Char('0')).toUpper();
        const QString cls = colorClass.value(hexc);
        const double cx = d.x + d.r, cy = d.y + d.r;
        svg << QString(R"(  <circle class="%1" cx="%2" cy="%3" r="%4"/>)")
                  .arg(cls).arg(cx,0,'f',6).arg(cy,0,'f',6).arg(d.r,0,'f',6);
    }
    svg << " </g>";
    svg << "</svg>";

    QFile f(file);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        ts.setEncoding(QStringConverter::Utf8);
        ts << svg.join('\n');
        f.close();
        statusBar()->showMessage(QString("Desen SVG olarak kaydedildi: %1").arg(file));
    } else {
        QMessageBox::critical(this, "Hata", "Dosya yazılamadı.");
    }
}

void MainWindow::onColorItemClicked(QListWidgetItem *item) {
    const int idx = ui->renkLmt->row(item);
    if (idx < 0 || idx >= PALETTE_.size()) return;
    const QColor c = QColorDialog::getColor(PALETTE_[idx], this);
    if (c.isValid()) {
        PALETTE_[idx] = c;
        updateColorList();
        updateDotItemBrushes();
        statusBar()->showMessage(QString("Renk %1 değiştirildi").arg(idx+1));
    }
}

void MainWindow::mergeColorsFromSlider() {
    if (PALETTE_.isEmpty()) return;
    const int tol = ui->RenkBirTol->value();
    QVector<QColor> cols = PALETTE_;
    if (tol > 0) mergeCloseColors(cols, tol);
    PALETTE_ = cols;
    updateColorList();
    updateDotItemBrushes();
    statusBar()->showMessage(QString("Renkler birleş
