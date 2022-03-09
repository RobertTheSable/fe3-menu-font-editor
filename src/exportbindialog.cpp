#include "exportbindialog.h"
#include "ui_exportbindialog.h"
#include <QFileDialog>

ExportBinDialog::ExportBinDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportBinDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Drawer |Qt::MSWindowsFixedSizeDialogHint);
    this->setFixedSize(this->size());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    exportExtraTiles = false;
    saveAddresses = true;
    write_directory = tr("");
    tileset_bin = tr("");
    tiles_bin = tr("");
    palette_bin = tr("");
}

ExportBinDialog::~ExportBinDialog()
{
    delete ui;
}

void ExportBinDialog::on_pushButton_clicked()
{
    QString dirname = QFileDialog::getExistingDirectory(this);
    if(dirname != "")
    {
        write_directory = dirname;
        ui->directoryLineEdit->setText(dirname);


        ui->tileSetLineEdit->setEnabled(true);
        ui->tilesetCheckBox->setEnabled(true);
        ui->paletteLineEdit->setEnabled(true);
        ui->palettesCheckBox->setEnabled(true);
        ui->rawTileLineEdit->setEnabled(true);
        ui->tilesCheckBox->setEnabled(true);

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }

}

bool ExportBinDialog::getSaveAddresses() const
{
    return saveAddresses;
}

QString ExportBinDialog::getPalette_bin() const
{
    return palette_bin;
}

QString ExportBinDialog::getTiles_bin() const
{
    return tiles_bin;
}

QString ExportBinDialog::getTileset_bin() const
{
    return tileset_bin;
}

QString ExportBinDialog::getWrite_directory() const
{
    return write_directory;
}

void ExportBinDialog::on_tileSetLineEdit_editingFinished()
{
    tileset_bin = ui->directoryLineEdit->text()+ QDir::separator() +ui->tileSetLineEdit->text();
}

void ExportBinDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    saveAddresses = ui->saveAddressesCheckBox->isChecked();
    if(!ui->tilesetCheckBox->isChecked())
    {
        tileset_bin = tr("");
    }
    else
    {
        tileset_bin = ui->directoryLineEdit->text()+ QDir::separator() +ui->tileSetLineEdit->text();
    }
    if(!ui->tilesCheckBox->isChecked())
    {
        tiles_bin = tr("");
    }
    else
    {
        tiles_bin = ui->directoryLineEdit->text()+ QDir::separator() + ui->rawTileLineEdit->text();
    }
    if(!ui->palettesCheckBox->isChecked())
    {
        palette_bin = tr("");
    }
    else
    {
        palette_bin = ui->directoryLineEdit->text()+ QDir::separator() + ui->paletteLineEdit->text();
    }
}
