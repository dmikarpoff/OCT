#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScopedPointer>
#include <QMainWindow>

#include <vector>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void layerBoundsChanged(int max_z, int layer_idx);

private:
    void loadSettings();
    void saveSettings();
    void createMenus();
    void updateButtonEnabled();
    void updateImage();
    void updateListOfLayerWidget();
    void reinitLayerBounds();

    void onAddModelLayer();
    void onRemoveModelLayer();
    void onGenerateClicked();
    void onExportFile();

    QScopedPointer<Ui::MainWindow> ui;
    std::vector<double> raw_data_;
    size_t img_width_ = 0;
    size_t img_height_ = 0;
};
#endif // MAINWINDOW_H
