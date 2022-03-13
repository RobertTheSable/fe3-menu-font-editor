#ifndef QBGTILESET_H
#define QBGTILESET_H
#include <QDir>
#include <QString>
#include <QDataStream>
#include <QPixmap>
#include <QColor>
#include <vector>
#include "undostack.h"

class BGTileSet
{
private:
    static const int TILESET_ENTRY_SIZE = 4;
    std::stack<UndoStack::UndoEntry> undo_stack, redo_stack;
    struct ActiveBGTile{
        int num;
        uchar data [0x20];
    };
    unsigned short m_BG2TransferSize;
    uint active_tile, active_x, active_y;

    QColor tileHighlightColor, bgHighlightColor;

    std::vector<uchar> TileSet;
    std::vector<uchar> BGTileData;
    std::vector<std::vector<QColor>> palettes;
    unsigned int m_iTilesetAddress, m_iBGTilesAddress, m_iPaletteAddress;
    unsigned int m_BitDepth;
    int tWidth, tHeight;
    unsigned int m_TilesetSize;
    unsigned short m_DefaultTile;
    unsigned int m_BGTileRowSize;

    //QFile m_RomFile;
    //int readAddressFromTable(int index, int data, int entrysize = 2);
    //int getChapterData()
    void generateDefaulPalettes();
    //int getActiveTileIndex(int tilenum);
public:
    enum TileQuadrant{UPPER_LEFT = 0, LOWER_LEFT = 1, UPPER_RIGHT = 2, LOWER_RIGHT = 3};

    BGTileSet()=default;
    BGTileSet(
        const std::vector<uchar>& tileset_data,
        const std::vector<uchar>& bg_tile_data,
        const std::vector<uchar>& pallete_data,
        int width,
        int height,
        unsigned int bitDepth,
        int tileCount,
        int paletteIndex,
        unsigned short defaultTile
    );

    QPixmap getTileSetPixmap(int scale, bool highlight = true);
    QPixmap getBGTilePixmap(int tilenum, bool vFlip, bool hFlip, int palette_number, int scale);
    QPixmap getBGTilesPixmap(bool vFlip, bool hFlip, int palette_number, int scale);
    QPixmap getTilePixmap(uint tilenum, int scale, bool highlight = true);
    QPixmap getActiveTilePixmap(int scale, bool whole = true);
    QPixmap getPalettePixmap(uint palette_num, int scale);

    QColor getPaletteColor(uint palette, uint color) const;
    QColor getTileHighlightColor() const;
    void setTileHighlightColor(const QColor &value);
    QColor getBgHighlightColor() const;
    void setBgHighlightColor(const QColor &value);


    unsigned int getTilesetAddress() const;
    unsigned int getBGTilesAddress() const;
    unsigned int getPaletteAddress() const;

    int getTileData(int index, int x, int y) const;
    int getActiveTileData() const;
    int getActive_x() const;
    void setActiveQuarter(int x, int y);
    int getActive_y() const;
    int getActive_tile() const;
    void setActive_tile(uint value);
    int getActivePaletteNumber() const;

    void setActiveTileData(uint value, bool vFlip, bool hFlip, uint palette);
    void setActiveTilePixel(int x, int y, int value);
    void copyBGTile(uint old_index, uint new_index);
    void copyTile(uint old_index, uint new_index);

    void undoLastEdit();
    void redoLastEdit();

    void exportData(int mode, uchar*& dest);
    static const int TILESET_MODE = 0;
    static const int TILE_MODE = 1;
    static const int EXTRA_TILE_MODE = 2;
    static const int PALETTE_MODE = 4;
    int getBGTileSize() const;
    int getTileSetSize() const;
};

#endif // QBGTILESET_H
