#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QErrorMessage>
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <vector>

void copyQByteArrayToVector(const QByteArray& byteArray, std::vector<uchar>& vector)
{
    std::copy(byteArray.begin(), byteArray.end(), std::back_inserter(vector));
}

//BGTileSet testTileset()
//{
//    QFile tilesetFile("/home/robert/QT/tileset.bin");
//    tilesetFile.open(QIODevice::ReadOnly);
//    QFile tilesFile("/home/robert/QT/tiles.bin");
//    tilesFile.open(QIODevice::ReadOnly);
//    QFile palettes("/home/robert/QT/palettes.bin");
//    palettes.open(QIODevice::ReadOnly);
//    std::vector<uchar> tilesetContent, tilesContent, paletteContent;
//    copyQByteArrayToVector(tilesetFile.readAll(), tilesetContent);
//    copyQByteArrayToVector(tilesFile.readAll(), tilesContent);
//    copyQByteArrayToVector(palettes.readAll(), paletteContent);

//    auto tileset = BGTileSet(tilesetContent, tilesContent, paletteContent, 256, 144, 2, 0x11B, 0, 0x20FE);
//    auto pixmap = tileset.getTileSetPixmap(8, false);
//    QFile picFile("/home/robert/QT/test.png");
//    picFile.open(QIODevice::WriteOnly);
//    pixmap.save(&picFile, "PNG");
//    return tileset;
//}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    highlightColor(QColor(0,0,0,0)),
    current_palette_color(0),
    switching(false)
{
    ui->setupUi(this);

    ui->tileEditLabel->setUnitSizes(8, 8);
    ui->tileEditLabel->setScale(4);
    ui->rawTileEditLabel->setUnitSizes(1, 1);
    ui->rawTileEditLabel->setScale(ui->zoomBGTileSpinBox->value());
    ui->rawTilesLabel->setUnitSizes(8, 8);
    ui->rawTilesLabel->setScale(2);
    ui->paletteLabel->setScale(3);
    ui->paletteLabel->setUnitSizes(8, 8);
    connect(ui->tileSetLabel, &TileSetLabel::clicked, this, &MainWindow::setTileFocus);
    connect(ui->tileEditLabel, &TileSetLabel::clicked, this, &MainWindow::setBGTileFromQuadrant);
    connect(ui->rawTilesLabel, &TileSetLabel::double_clicked, this, &MainWindow::selectTileFromBank);
    connect(ui->paletteLabel, &TileSetLabel::clicked, this, &MainWindow::selectColor);
    connect(ui->rawTileEditLabel, &TileSetLabel::clicked, this, &MainWindow::editActiveTile);
    connect(ui->rawTilesLabel, &TileSetLabel::released, this, &MainWindow::dragBGTile);
    connect(ui->tileSetLabel, &TileSetLabel::released, this, &MainWindow::dragMainTile);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::show()
{
    QMainWindow::show();
    auto iniPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, INI_DIR, QStandardPaths::LocateDirectory);
//    if (iniPath == "") {
//        QMessageBox::critical(
//                    NULL,
//                    tr("Error"),
//                    tr("The ini folder was not found - this folder is needed for the application to function.")
//                    );
//        close();
//    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open ROM", QDir::currentPath(), tr("SNES Roms (*.sfc *.smc)"));
    if(filename != "")
    {
        std::string text = filename.toStdString();
        auto iniPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, INI_DIR, QStandardPaths::LocateDirectory);
        m_RomMap = RomMap(text, iniPath.toStdString());
        if(m_RomMap.getMyState() == RomMap::rom_ok)
        {
            std::vector<uchar> tilesetContent, tilesContent, paletteContent;
            m_Tileset = BGTileSet(tilesetContent, tilesContent, paletteContent, 256, 144, 2, 0x11B, 0, 0x20FE);
            ui->tileSetLabel->setEnabled(true);
            ui->zoomBGTileSpinBox->setEnabled(true);
            ui->actionExport->setEnabled(true);
            ui->actionHighlight_Same_BG_Tile->setEnabled(true);
            ui->actionRedo->setEnabled(true);
            ui->actionUndo->setEnabled(true);
            ui->actionOpen_From_Files->setEnabled(false);
            ui->actionOpen->setEnabled(false);
            ui->actionExport_PNG->setEnabled(true);
            updateDisplayData();
            updatePaletteDisplay();
        }
        else
        {
            switch (m_RomMap.getMyState()) {
            case RomMap::bad_header:
                QMessageBox::warning(this, tr("Error Opening ROM"), tr("ROM header is malformed."));
                break;
            case RomMap::not_supported:
                QMessageBox::warning(this, tr("Error Opening ROM"), tr("Current ROM is not supported."));
                break;
            case RomMap::malformed_name:
                QMessageBox::warning(this, tr("Error Opening ROM"), tr("Internal header appears malformed."));
                break;
            case RomMap::no_file:
                QMessageBox::warning(this, tr("Error Opening ROM"), tr("Could not open ROM file."));
                break;
            default:
                QMessageBox::warning(this, tr("Error Opening ROM"), tr("Unidentified error opening ROM."));
                break;
            }
        }
    }
}

void MainWindow::setTileFocus(int x, int y)
{
    switching = true;
    if(x < 32 && y < 9)
    {
        int index = y*32 +x;
        m_Tileset.setActive_tile(index);
        updateBGTileDisplay();
        updateTileDisplay();
        updateBGTilesetDisplay();
    }
    switching = false;
}

void MainWindow::selectTileFromBank(int x, int y)
{
    int tile_index = x+(y*16);
    m_Tileset.setActiveTileData(tile_index, ui->tileVFlipCheckBox->isChecked(), ui->tileHFlipCheckBox->isChecked(), ui->tilePalletteSpinBox->value());
    updateTileDisplay();
    updateBGTileDisplay();
    updateBGTilesetDisplay();
    updateTilesetDisplay();
}

void MainWindow::setBGTileFromQuadrant(int x, int y)
{
    switching = true;
    if(x < 1 && y < 2)
    {
        m_Tileset.setActiveQuarter(x,y);
        updateBGTileDisplay();
        updateTilesetDisplay();
        updateBGTilesetDisplay();
    }
    switching = false;
}

void MainWindow::editActiveTile(int x, int y)
{
    if(x < 8 && y < 8)
    {
        m_Tileset.setActiveTilePixel(x,y,current_palette_color);
        updateBGTilesetDisplay();
        updateTileDisplay();
        updateBGTileDisplay();
        updateTilesetDisplay();
    }
}

void MainWindow::selectColor(int x, int y)
{
    current_palette_color = x+(y*8);
    updatePaletteDisplay();
}

void MainWindow::dragBGTile(int old_x, int old_y, int new_x, int new_y)
{
    m_Tileset.copyBGTile(old_x + (old_y*16), new_x + (new_y*16));
    updateBGTilesetDisplay();
    m_Tileset.setActive_tile(m_Tileset.getActive_tile());
    updateTilesetDisplay();
    updateTileDisplay();
    updateBGTileDisplay();
}

void MainWindow::dragMainTile(int old_x, int old_y, int new_x, int new_y)
{
    m_Tileset.copyTile(old_x + (old_y*32), new_x + (new_y*32));
    updateBGTilesetDisplay();
    updateTilesetDisplay();
    updateTileDisplay();
    updateBGTileDisplay();
}

void MainWindow::on_chapterSetButton_clicked()
{
    updateDisplayData();
}

void MainWindow::on_chapterSpinBox_editingFinished()
{
    updateDisplayData();
}

void MainWindow::updateDisplayData()
{
    updateTilesetDisplay();
    updateBGTilesetDisplay();
    updateTileDisplay();
    updateBGTileDisplay();
}

void MainWindow::updateTileDisplay()
{
    ui->tileEditLabel->setEnabled(true);
    ui->tilePalletteSpinBox->setEnabled(true);
    ui->tileNumberEdit->setEnabled(true);
    ui->tileHFlipCheckBox->setEnabled(true);
    ui->tileVFlipCheckBox->setEnabled(true);
    ui->tileEditLabel->setPixmap(m_Tileset.getActiveTilePixmap(ui->tileEditLabel->getScale()));
    ui->tileSetLabel->setPixmap(m_Tileset.getTileSetPixmap(2));
}

void MainWindow::updateBGTileDisplay(bool reload)
{
    if(reload) {
        int tile_data = m_Tileset.getActiveTileData();//getTileData(index, x, y);
        int tile_num = tile_data&0x03FF;
        int palette_num = (tile_data&0x1C00)>>10;
        bool vFlip = (tile_data&0x8000)!= 0;
        bool hFlip = (tile_data&0x4000)!= 0;
        ui->tileNumberEdit->setText(QString::number(tile_num));
        ui->tilePalletteSpinBox->setValue(palette_num);
        ui->tileHFlipCheckBox->setChecked(hFlip);
        ui->tileVFlipCheckBox->setChecked(vFlip);
    }
    ui->rawTileEditLabel->setPixmap(m_Tileset.getActiveTilePixmap(ui->rawTileEditLabel->getScale(), false));//getBGTilePixmap(tile_num,vFlip, hFlip,palette_num, ui->rawTileEditLabel->getScale()));
}

void MainWindow::updateTilesetDisplay()
{
    ui->tileSetLabel->setPixmap(m_Tileset.getTileSetPixmap(2));
}

void MainWindow::updateBGTilesetDisplay()
{
    int current_palette = ui->tilePalletteSpinBox->value();
    ui->rawTilesLabel->setPixmap(m_Tileset.getBGTilesPixmap(false, false, current_palette, ui->rawTilesLabel->getScale()));
}

void MainWindow::updatePaletteDisplay()
{
    int current_palette = ui->tilePalletteSpinBox->value();
    ui->paletteLabel->setPixmap(m_Tileset.getPalettePixmap(current_palette,ui->paletteLabel->getScale()));
    QPixmap palette_color = QPixmap(48,48);
    QPainter p(&palette_color);
    p.fillRect(0,0,48,48,m_Tileset.getPaletteColor(current_palette,current_palette_color));
    p.drawRect(0,0,47,47);
    ui->selectedColorLabel->setPixmap(palette_color);

}

void MainWindow::on_actionExport_triggered()
{
    ExportBinDialog export_dialogue;
    if(export_dialogue.exec() == QDialog::Accepted)
    {
        //QMessageBox::information(this, tr("Test"),export_dialogue.getExtra_tiles_bin());
        if(export_dialogue.getTileset_bin()!= "")
        {
            QString filename = export_dialogue.getTileset_bin();
            QFile outfile(filename);
            if(!outfile.open(QFile::WriteOnly))
            {
                QMessageBox::warning(this, tr("Error opening file"),tr("Could not open ") + filename + tr(" for writing."));
            }
            else
            {
                uchar* output = new uchar[m_Tileset.getTileSetSize()];
                m_Tileset.exportData(BGTileSet::TILESET_MODE, output);
                outfile.write((char*)output,m_Tileset.getTileSetSize());
                outfile.flush();
                outfile.close();
                delete[] output;
            }
        }
        if(export_dialogue.getTiles_bin() != "")
        {
            QString filename = export_dialogue.getTiles_bin();
            QFile outfile(filename);
            if(!outfile.open(QFile::WriteOnly))
            {
                QMessageBox::warning(this, tr("Error opening file"),tr("Could not open ") + filename + tr(" for writing."));
            }
            else
            {
                uchar* output = new uchar[m_Tileset.getBGTileSize()];
                m_Tileset.exportData(BGTileSet::TILE_MODE, output);
                outfile.write((char*)output, m_Tileset.getBGTileSize());
                outfile.flush();
                outfile.close();
                delete[] output;
            }

        }
        if(export_dialogue.getPalette_bin() != "")
        {
            QString filename = export_dialogue.getPalette_bin();
            QFile outfile(filename);
            if(!outfile.open(QFile::WriteOnly))
            {
                QMessageBox::warning(this, tr("Error opening file"),tr("Could not open ") + filename + tr(" for writing."));
            }
            else
            {
                uint paletteSize = 512;
                uchar* output = new uchar[paletteSize];
                m_Tileset.exportData(BGTileSet::PALETTE_MODE, output);
                outfile.write((char*)output, paletteSize);
                outfile.flush();
                outfile.close();
                delete[] output;
            }
        }
    }
}

void MainWindow::on_zoomBGTileSpinBox_editingFinished()
{
    ui->rawTileEditLabel->setScale(ui->zoomBGTileSpinBox->value());
    updateBGTileDisplay();
}

void MainWindow::on_actionHighlight_Same_BG_Tile_toggled(bool arg1)
{
    if(arg1)
    {
        highlightColor = QColor(255,0,255,100);
    }
    else
    {
        highlightColor = QColor(0,0,0,0);
    }
    m_Tileset.setBgHighlightColor(highlightColor);
    updateTilesetDisplay();
}

void MainWindow::on_tileVFlipCheckBox_toggled(bool checked)
{
    if(!switching)
    {
        m_Tileset.setActiveTileData(ui->tileNumberEdit->text().toInt(),checked, ui->tileHFlipCheckBox->isChecked(), ui->tilePalletteSpinBox->value());
        updateBGTileDisplay();
        updateTileDisplay();
        updateTilesetDisplay();
    }
}

void MainWindow::on_tileHFlipCheckBox_toggled(bool checked)
{
    if(!switching)
    {
        m_Tileset.setActiveTileData(ui->tileNumberEdit->text().toInt(), ui->tileVFlipCheckBox->isChecked(), checked, ui->tilePalletteSpinBox->value());
        updateBGTileDisplay();
        updateTileDisplay();
        updateTilesetDisplay();
    }
}

void MainWindow::on_tilePalletteSpinBox_valueChanged(int arg1)
{
    if(!switching)
    {
        m_Tileset.setActiveTileData(ui->tileNumberEdit->text().toInt(), ui->tileVFlipCheckBox->isChecked(), ui->tileHFlipCheckBox->isChecked(), arg1);
        updateBGTileDisplay(false);
        updateTileDisplay();
        updateTilesetDisplay();
    }
    updatePaletteDisplay();
}

void MainWindow::on_actionUndo_triggered()
{
     m_Tileset.undoLastEdit();
     updateBGTilesetDisplay();
     updateTilesetDisplay();
     updateBGTileDisplay();
     updateTileDisplay();
}

void MainWindow::on_actionRedo_triggered()
{
    m_Tileset.redoLastEdit();
    updateBGTilesetDisplay();
    updateTilesetDisplay();
    updateBGTileDisplay();
    updateTileDisplay();
}

std::vector<uchar> getDataFromBinaryFile(QString filename)
{
    std::ifstream input_tileset(filename.toStdString(), std::ios::binary | std::ios::ate);
    input_tileset.unsetf(std::ios::skipws);
    uint tilesetFileSize = input_tileset.tellg();
    std::vector<uchar> tileset_data;
    tileset_data.reserve(tilesetFileSize);
    input_tileset.seekg(0, std::ios::beg);
    std::copy(std::istream_iterator<uchar>(input_tileset), std::istream_iterator<uchar>(), std::back_inserter(tileset_data));
    input_tileset.close();
    return tileset_data;
}



void MainWindow::quickOpen(const QString &tilesetPath, const QString &tilesPath, const QString &palettePath)
{
    if (QFile::exists(tilesetPath) && QFile::exists(tilesPath) && QFile::exists(palettePath)) {
        std::vector<uchar> tilesetData = getDataFromBinaryFile(tilesetPath);
        std::vector<uchar> tileData = getDataFromBinaryFile(tilesPath);
        std::vector<uchar> paletteData = getDataFromBinaryFile(palettePath);
        m_Tileset = BGTileSet(tilesetData, tileData, paletteData, 256, 144, 2, 0x11B, 0, 0x20FE);
        ui->tileSetLabel->setEnabled(true);
        ui->zoomBGTileSpinBox->setEnabled(true);
        ui->actionExport->setEnabled(true);
        ui->actionHighlight_Same_BG_Tile->setEnabled(true);
        ui->actionRedo->setEnabled(true);
        ui->actionUndo->setEnabled(true);
        ui->actionExport_PNG->setEnabled(true);
        updateDisplayData();
        updatePaletteDisplay();
        updateDisplayData();
        ui->tileSetLabel->setEnabled(true);
    }
}


void MainWindow::on_actionOpen_From_Files_triggered()
{
    OpenDirDialog opendir;
    if (opendir.exec() == QDialog::Accepted)
    {
        std::vector<uchar> tilesetData = getDataFromBinaryFile(opendir.getTileSetFile());
        std::vector<uchar> tileData = getDataFromBinaryFile(opendir.getTilesFile());
        std::vector<uchar> paletteData = getDataFromBinaryFile(opendir.getPaletteFile());
        m_Tileset = BGTileSet(tilesetData, tileData, paletteData, 256, 144, 2, 0x11B, 0, 0x20FE);
        ui->tileSetLabel->setEnabled(true);
        ui->zoomBGTileSpinBox->setEnabled(true);
        ui->actionExport->setEnabled(true);
        ui->actionHighlight_Same_BG_Tile->setEnabled(true);
        ui->actionRedo->setEnabled(true);
        ui->actionUndo->setEnabled(true);
        ui->actionExport_PNG->setEnabled(true);
        updateDisplayData();
        updatePaletteDisplay();
    }
}

void MainWindow::on_actionExport_PNG_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Output File", QDir::currentPath(), tr("PNG (*.png)"));
    if (filename != "") {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            m_Tileset.getTileSetPixmap(1, false).save(&file, "PNG");
            QMessageBox::information(this, tr("Success"), tr("Image exported to ") + filename);
        } else {
            QMessageBox::warning(this, tr("Error opening file"),tr("Could not open ") + filename + tr(" for writing."));
        }


    }
}
