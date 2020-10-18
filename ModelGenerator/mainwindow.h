#pragma once

#include <oct_utils/scan_data.h>

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
    oct::ScanData scan_data_;
};
