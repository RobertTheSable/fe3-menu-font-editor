#include "bgtileset.h"
#include "addresser/addresser.h"
#include <QPainter>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>

BGTileSet::BGTileSet(
        const std::vector<uchar>& tileset_data,
        const std::vector<uchar>& bg_tile_data,
        const std::vector<uchar>& pallete_data,
        int width,
        int height,
        unsigned int bitDepth,
        int tileCount,
        int paletteIndex,
        unsigned short defaultTile
) :
    active_tile(0), active_x(0), active_y(0), tileHighlightColor(QColor(0,0,255,100)), bgHighlightColor(QColor(0,0,0,0)),
    m_iTilesetAddress(0), m_iBGTilesAddress(0), m_iPaletteAddress(0),
    tWidth(width), tHeight(height), m_TilesetSize(tileCount), m_DefaultTile(defaultTile)
{
    BGTileData.reserve(bg_tile_data.size());
    std::copy(bg_tile_data.begin(), bg_tile_data.end(), std::back_inserter(BGTileData));

    TileSet.reserve(tileset_data.size());
    auto stop_itr = tileset_data.begin();
    stop_itr += (tileCount * TILESET_ENTRY_SIZE);
    std::copy(tileset_data.begin(), stop_itr, std::back_inserter(TileSet));

    int correctedBitDepth = bitDepth;
    if (correctedBitDepth != 2 && correctedBitDepth != 4) {
        correctedBitDepth = 2;
    }
    m_BitDepth = correctedBitDepth;
    m_BGTileRowSize = correctedBitDepth == 2 ? 0x10 : 0x20;

    int individualPaletteSize = correctedBitDepth == 2 ? 4 : 16;

    //generateDefaulPalettes();

    auto numberOfPAlettes = correctedBitDepth == 2 ? 64 : 16;
    palettes.reserve(numberOfPAlettes);
    for(int pal_num = paletteIndex; pal_num < numberOfPAlettes; pal_num++) {
        int pal_index = pal_num * individualPaletteSize;
        pal_index = (pal_num -  paletteIndex) * individualPaletteSize * 2;
        std::vector<QColor> palette;
        palette.reserve(individualPaletteSize);
        for(int color_num = 0; color_num < individualPaletteSize * 2; color_num+=2) {
            unsigned short color16 = (*(unsigned short*)&(pallete_data[pal_index + color_num]));
            int blue = (color16&0x7C00) >> 10;
            int green =(color16&0x3E0) >> 5;
            int red = (color16&0x1F);
            palette.push_back(QColor(red<<3, green<<3, blue<<3));
        }
        palettes.push_back(palette);
    }
//    m_iTilesetAddress = PCToLoROM(tileset_data - base_data, false);
//    m_iBGTilesAddress = PCToLoROM(bg_tile_data - base_data, false);
//    m_iPaletteAddress = PCToLoROM(pallete_data - base_data, false);
//    if (bg_tile_data_2 != nullptr) {
//        m_iExtraTilesAddress = PCToLoROM(bg_tile_data_2 - base_data, false);
//    }

}

unsigned int BGTileSet::getTilesetAddress() const
{
    return m_iTilesetAddress;
}

unsigned int BGTileSet::getBGTilesAddress() const
{
    return m_iBGTilesAddress;
}

unsigned int BGTileSet::getPaletteAddress() const
{
    return m_iPaletteAddress;
}

QColor BGTileSet::getTileHighlightColor() const
{
    return tileHighlightColor;
}

void BGTileSet::setTileHighlightColor(const QColor &value)
{
    tileHighlightColor = value;
}

QColor BGTileSet::getBgHighlightColor() const
{
    return bgHighlightColor;
}

void BGTileSet::setBgHighlightColor(const QColor &value)
{
    bgHighlightColor = value;
}

int BGTileSet::getBGTileSize() const
{
    return BGTileData.size();
}

int BGTileSet::getTileSetSize() const
{
    return TileSet.size();
}

void BGTileSet::generateDefaulPalettes()
{
    const char* defaultPalette = "resource/palettes.bin";
    std::ifstream inFile(defaultPalette, std::ios::in|std::ios::binary);
    for (auto &palette: palettes)
    {
        for (auto &color: palette)
        {
            ushort snesColor;
            inFile.read((char *)&snesColor,sizeof(short));
            int blue = (snesColor&0x7C00) >> 10;
            int green =(snesColor&0x3E0) >> 5;
            int red = (snesColor&0x1F);
            color = QColor(red<<3, green<<3, blue<<3);
            //cout << hex << color[i] << endl;
        }
    }
    inFile.close();
}

QPixmap BGTileSet::getTileSetPixmap(int scale, bool highlight)
{
    QPixmap pix(tWidth*scale, tHeight*scale);
    QPainter p(&pix);
    uint maxCounter = (tWidth/8) * (tHeight/16);
    for(uint i = 0; i < maxCounter; i++) {
        int x = i%32;
        int y = i/32;
        p.drawPixmap(x*8*scale, y*16*scale, getTilePixmap(i,scale, highlight));

    }
    return pix;
}

QPixmap BGTileSet::getTilePixmap(uint tilenum, int scale, bool highlight)
{
    QPixmap pix(8*scale,16*scale);
    //int test = TileSet.size()/8;
    if(tilenum < m_TilesetSize)
    {
        QPainter p(&pix);
        for(int i = 0; i < 4; i +=2)
        {
            int base_location = (tilenum*4)+i;
            unsigned short tileData = *(unsigned short*)&TileSet[base_location];
            if (i == 2 && tileData == 0xFFFF) {
                tileData = m_DefaultTile;
            }
            int tile_num = tileData&0x03FF;
            int palette_num = (tileData&0x1C00)>>10;
            bool vFlip = (tileData&0x8000)!= 0;
            bool hFlip = (tileData&0x4000)!= 0;
            int x = i/4;
            int y = (i/2)%2;
            p.drawPixmap(x*scale*8,y*scale*8, getBGTilePixmap(tile_num, vFlip,hFlip,palette_num,scale));

            //QColor test = bgHighlightColor;
            unsigned short active_num = (*( (unsigned short*)
                                           &TileSet[(active_tile * TILESET_ENTRY_SIZE)+((active_y + (active_x<<1))*2)]
                                         ) )&0x3FF;
            if(bgHighlightColor.alpha() != 0 && tile_num == active_num && highlight)
            {
                p.fillRect(x*scale*8,y*scale*8,8*scale,8*scale,bgHighlightColor);
            }
            else if(tileHighlightColor.alpha() != 0 && tilenum == active_tile && highlight)
            {
                p.fillRect(x*scale*8,y*scale*8,8*scale,8*scale,tileHighlightColor);
            }
        }
    } else {
        QPainter p(&pix);
        for(int i = 0; i < 4; i +=2)
        {
            unsigned short tileData = m_DefaultTile;
            if (i == 2 && tileData == 0xFFFF) {
                tileData = m_DefaultTile;
            }
            int tile_num = tileData&0x03FF;
            int palette_num = (tileData&0x1C00)>>10;
            bool vFlip = (tileData&0x8000)!= 0;
            bool hFlip = (tileData&0x4000)!= 0;
            int x = i/4;
            int y = (i/2)%2;
            p.drawPixmap(x*scale*8,y*scale*8, getBGTilePixmap(tile_num, vFlip,hFlip,palette_num,scale));

            //QColor test = bgHighlightColor;
            unsigned short active_num = (*( (unsigned short*)
                                           &TileSet[(active_tile * TILESET_ENTRY_SIZE)+((active_y + (active_x<<1))*2)]
                                         ) )&0x3FF;
            if(bgHighlightColor.alpha() != 0 && tile_num == active_num && highlight)
            {
                p.fillRect(x*scale*8,y*scale*8,8*scale,8*scale,bgHighlightColor);
            }
            else if(tileHighlightColor.alpha() != 0 && tilenum == active_tile && highlight)
            {
                p.fillRect(x*scale*8,y*scale*8,8*scale,8*scale,tileHighlightColor);
            }
        }
    }
    return pix;
}

QPixmap BGTileSet::getBGTilePixmap(int tilenum, bool vFlip, bool hFlip, int palette_number, int scale)
{
    QPixmap pix(8*scale,8*scale);
    QPainter p(&pix);
    int tile_index = 8 * m_BitDepth * tilenum;
    std::vector<QColor> colors;
    for(int y = 0; y < 16; y += 2)
    {
        int y_off = tile_index + y;
        for(int x = 0; x < 8; x++)
        {
            int bitmask = (0x80 >> x);
            int shift = 7-x;
            int calculatedColorIndex  = (BGTileData[y_off]&bitmask) >> shift;
            calculatedColorIndex |= ((BGTileData[y_off+1]&bitmask) >> shift) << 1;
            if (m_BitDepth == 4) {
                calculatedColorIndex |= ((BGTileData[y_off+0x10]&bitmask) >> shift) << 2;
                calculatedColorIndex |= ((BGTileData[y_off+0x11]&bitmask) >> shift) << 3;
            }

            colors.push_back(getPaletteColor(palette_number, calculatedColorIndex));
        }
    }
    int y_pos = 0;
    int y_off = 1;
    if(vFlip)
    {
        y_pos = 7;
        y_off = -1;
    }
    int x_start = 0;
    int x_off = 1;
    if(hFlip)
    {
        x_start = 7;
        x_off = -1;
    }
    for(int y = 0; y < 8; y ++)
    {
        int x_pos = x_start;
        for(int x = 0; x <8 ; x++)
        {
            //p.setPen(colors[(y_pos*8)+x_pos]);
            p.fillRect(x*scale,y*scale,scale,scale,colors[(y_pos*8)+x_pos]);
            x_pos += x_off;
        }
        y_pos += y_off;
    }
    return pix;
}

QPixmap BGTileSet::getBGTilesPixmap(bool vFlip, bool hFlip, int palette_number, int scale)
{
    int numberOfTiles = BGTileData.size() / (m_BitDepth*8);
    QPixmap pix(0x80*scale, (numberOfTiles /2)  *scale);

    QPainter p(&pix);
    for(int i = 0; i < numberOfTiles; i++)
    {
        int x = i%0x10;
        int y = i/0x10;
        p.drawPixmap(x*scale*8,y*scale*8, getBGTilePixmap(i, vFlip,hFlip,palette_number,scale));
        unsigned short activeTileData = (*(unsigned short*)&TileSet[(active_tile * TILESET_ENTRY_SIZE)+(active_y + (active_x << 1))*2]);
        if (activeTileData == 0xFFFF) {
            activeTileData = m_DefaultTile;
        }
        if(tileHighlightColor.alpha() != 0 && i == (activeTileData&0x03FF))
        {
            p.fillRect(x*scale*8,y*scale*8,8*scale,8*scale,tileHighlightColor);
        }
    }
    return pix;
}

QPixmap BGTileSet::getActiveTilePixmap(int scale, bool whole)
{
    QPixmap pix(8*scale,8*scale);
    unsigned int tile_location = active_tile * 4;
    if(whole)
    {
        pix = QPixmap(8*scale,16*scale);
        QPainter p(&pix);
        uint old_x = active_x;
        uint old_y = active_y;

        for(int i = 0; i < 4; i +=2)
        {
            //int base_location = (tilenum*8)+i;
            unsigned short tileData = *((unsigned short*)&TileSet[tile_location+i]);//activeTileData[i/2];
            if (i == 2 && tileData == 0xFFFF) {
                tileData = m_DefaultTile;
            }
            int tile_num = tileData&0x03FF;
            int palette_num = (tileData&0x1C00)>>10;
            bool vFlip = (tileData&0x8000)!= 0;
            bool hFlip = (tileData&0x4000)!= 0;
            active_x = i/4;
            active_y = (i/2)%2;

            p.drawPixmap(active_x*scale*8,active_y*scale*8, getBGTilePixmap(tile_num, vFlip,hFlip,palette_num,scale));
        }
        active_x = old_x;
        active_y = old_y;
    }
    else
    {
        unsigned short active_data = *((unsigned short*)&TileSet[tile_location+((active_y + (active_x<<1))*2)]);//activeTileData[active_y + (active_x<<1)];
        if (active_y == 1 && active_data == 0xFFFF) {
            active_data = m_DefaultTile;
        }
        int tile_num = active_data&0x03FF;
        int palette_num = (active_data&0x1C00)>>10;
        bool vFlip = (active_data&0x8000)!= 0;
        bool hFlip = (active_data&0x4000)!= 0;
        pix = getBGTilePixmap(tile_num,vFlip,hFlip,palette_num,scale);
    }
    return pix;
}

QColor BGTileSet::getPaletteColor(uint palette, uint color) const
{
    uint maxPalette = 32 / m_BitDepth;
    if(palette < maxPalette && (color < (8 * m_BitDepth))) {
        return palettes[palette][color];
    }
    return QColor();
}

QPixmap BGTileSet::getPalettePixmap(uint palette_num, int scale)
{
    QPixmap pix(32*scale,8*scale);
    if(palette_num < palettes.size())
    {
        QPainter p(&pix);
        //unsigned short* current_palette = palettes[palette_num];
        for(uint i = 0; i < palettes[i].size(); i++)
        {
            int x = (i%8)*8*scale;
            int y = (i/8)*8*scale;
            QColor p_color  = getPaletteColor(palette_num, i);
            p.fillRect(x,y,8*scale,8*scale,p_color);
            p.setPen(QColor(0,0,0));
            p.drawRect(x,y,(8*scale)-1,(8*scale)-1);

        }
    }
    return pix;
}

int BGTileSet::getTileData(int index, int x, int y) const
{
    int tile_quadrant  = y | (x <<1);
    int location = (index*8) + (tile_quadrant*2);
    return (*(unsigned short*)&TileSet[location]);
}

int BGTileSet::getActiveTileData() const
{
    int return_val = -1;
    if((active_x == 0) && (active_y == 0 || active_y == 1))
    {
        int tile_quadrant  = (active_y | (active_x <<1))*2;
        unsigned int tile_location = active_tile * TILESET_ENTRY_SIZE;
        return_val = *((unsigned short*)&TileSet[tile_location+tile_quadrant]);//activeTileData[tile_quadrant];
    }
    return return_val;
}
int BGTileSet::getActive_x() const
{
    return active_x;
}

void BGTileSet::setActiveQuarter(int x, int y)
{
    active_x = x;
    active_y = y;
}

int BGTileSet::getActive_y() const
{
    return active_y;
}

int BGTileSet::getActive_tile() const
{
    return active_tile;
}

void BGTileSet::setActive_tile(uint value)
{
    if (value < m_TilesetSize) {
        if(active_tile != value)
        {
            setActiveQuarter(0,0);
        }
        active_tile = value;
    }
}

int BGTileSet::getActivePaletteNumber() const
{
    int activeData = getActiveTileData();
    return (activeData&0x1C00)>>10;
}

void BGTileSet::setActiveTileData(uint new_value, bool vFlip, bool hFlip, uint palette)
{
    unsigned int value = new_value;
    std::vector<uchar>::iterator activeTileData = TileSet.begin() + (active_tile*TILESET_ENTRY_SIZE);
    UndoStack::UndoEntry u = {
        UndoStack::TILESET_EDIT,
        active_tile*TILESET_ENTRY_SIZE,
        std::vector<uchar>(activeTileData, activeTileData+TILESET_ENTRY_SIZE)
    };
    undo_stack.push(u);
    while(!redo_stack.empty())
    {
        redo_stack.pop();
    }
    if(value <= 0x3FF)
    {
        unsigned short tile_data = value;
        //int old_tile = activeTileData[active_y + (active_x<<1)]&0x3FF;
        tile_data |= (palette << 10);
        if(vFlip)
        {
            tile_data |= 0x8000;
        }
        if(hFlip)
        {
            tile_data |= 0x4000;
        }
        auto activeTileIndex = (active_y + (active_x<<1))*2;
        activeTileData[activeTileIndex] = tile_data&0xFF;
        activeTileData[activeTileIndex + 1] = (tile_data>>8)&0xFF;
    }
}

void BGTileSet::setActiveTilePixel(int x, int y, int value)
{
    auto searchedTileIndex = (active_tile*TILESET_ENTRY_SIZE)+((active_y + (active_x<<1))*2);
    unsigned short tileData = *((unsigned short*)&TileSet[searchedTileIndex]);
    ushort tile_num = tileData & 0x3FF;
    std::vector<uchar>::iterator activeBGData;
    activeBGData = BGTileData.begin()+(tile_num*m_BGTileRowSize);

    bool vFlip = (tileData&0x8000)!= 0;
    bool hFlip = (tileData&0x4000)!= 0;
    int y_off = y;
    if(vFlip)
    {
        y_off = 7-y;
    }
    int x_off = x;
    if(hFlip)
    {
        x_off = 7-x;
    }

    int bitmask = (0x80 >> x_off);
    uchar push_value = 0;
    int shift = 7-x_off;
    push_value |= (activeBGData[y_off*2]&bitmask) >> shift;
    push_value |= ((activeBGData[(y_off*2)+1]&bitmask) >> shift) << 1;
    if (m_BitDepth == 4) {
        push_value |= ((activeBGData[(y_off*2)+0x10]&bitmask) >> shift) << 2;
        push_value |= ((activeBGData[(y_off*2)+0x11]&bitmask) >> shift) << 3;
    }

    uchar pixel_num = x_off+(y_off*8);
    UndoStack::UndoEntry u = {
        UndoStack::PIXEL_EDIT,
        tile_num,
        std::vector<uchar>({pixel_num, push_value})
    };
    undo_stack.push(u);
    while(!redo_stack.empty())
    {
        redo_stack.pop();
    }
    if((value&0b0001)!= 0)
    {
        activeBGData[y_off*2] |= (bitmask);
    }
    else
    {
        activeBGData[y_off*2] &= ~(bitmask);
    }
    if((value&0b0010)!= 0)
    {
        activeBGData[(y_off*2)+1] |= (bitmask);
    }
    else
    {
        activeBGData[(y_off*2)+1] &= ~(bitmask);
    }
    if (m_BitDepth == 4) {
        if((value&0b0100)!= 0)
        {
            activeBGData[(y_off*2)+0x10] |= (bitmask);
        }
        else
        {
            activeBGData[(y_off*2)+0x10] &= ~(bitmask);
        }
        if((value&0b1000)!= 0)
        {
            activeBGData[(y_off*2)+0x11] |= (bitmask);
        }
        else
        {
            activeBGData[(y_off*2)+0x11] &= ~(bitmask);
        }
    }
}

void BGTileSet::copyBGTile(uint old_tile_number, uint new_tile_number)
{
    unsigned int old_tile_index = (old_tile_number * m_BGTileRowSize);
    unsigned int new_tile_index = (new_tile_number * m_BGTileRowSize);
    std::vector<uchar>::iterator old_it;
    std::vector<uchar>::iterator new_it;

    old_it = BGTileData.begin()+old_tile_index;

    new_it = BGTileData.begin()+new_tile_index;

    UndoStack::UndoEntry u = {
        UndoStack::TILE_EDIT,
        new_tile_number,
        std::vector<uchar>(new_it, new_it+m_BGTileRowSize)
    };
    undo_stack.push(u);
    while(!redo_stack.empty())
    {
        redo_stack.pop();
    }
    std::copy(old_it, old_it+m_BGTileRowSize, new_it);
}

void BGTileSet::copyTile(uint old_index, uint new_index)
{
    unsigned int old_tile_index = (old_index * TILESET_ENTRY_SIZE);
    unsigned int new_tile_index = (new_index * TILESET_ENTRY_SIZE);
    std::vector<uchar>::iterator old_it = (TileSet.begin() + old_tile_index);
    std::vector<uchar>::iterator new_it = (TileSet.begin() + new_tile_index);
    UndoStack::UndoEntry u = {
        UndoStack::TILESET_EDIT,
        new_index*TILESET_ENTRY_SIZE,
        std::vector<uchar>(new_it, new_it+TILESET_ENTRY_SIZE)
    };
    undo_stack.push(u);
    while(!redo_stack.empty())
    {
        redo_stack.pop();
    }
    std::copy(old_it, old_it+TILESET_ENTRY_SIZE, new_it);
}

void BGTileSet::undoLastEdit()
{
    if(!undo_stack.empty())
    {
        if((undo_stack.top().data_type & UndoStack::TILESET_EDIT) != 0)
        {
            uint location = undo_stack.top().change_index;
            UndoStack::UndoEntry u = {
                undo_stack.top().data_type,
                undo_stack.top().change_index,
                std::vector<uchar>(TileSet.begin()+location, TileSet.begin()+location+TILESET_ENTRY_SIZE)
            };
            redo_stack.push(u);
            std::copy(undo_stack.top().data.begin(), undo_stack.top().data.end(), TileSet.begin()+location);
        }
        else if((undo_stack.top().data_type & UndoStack::TILE_EDIT) != 0)
        {
             uint location = (undo_stack.top().change_index * m_BGTileRowSize);
             std::vector<uchar>::iterator data_begin;
             data_begin = BGTileData.begin() + location;

             UndoStack::UndoEntry u = {
                 undo_stack.top().data_type,
                 undo_stack.top().change_index,
                 std::vector<uchar>(data_begin, data_begin+m_BGTileRowSize)
             };
             redo_stack.push(u);
             std::copy(undo_stack.top().data.begin(), undo_stack.top().data.end(), data_begin);
        }
        else if((undo_stack.top().data_type & UndoStack::PIXEL_EDIT) != 0)
        {
            uint location = (undo_stack.top().change_index * m_BGTileRowSize);
            std::vector<uchar>::iterator data_begin;

            data_begin = BGTileData.begin() + location;
            uchar pixel_num = undo_stack.top().data[0];
            uchar current_data = 0;
            int bitmask = 0x80 >> (pixel_num%8);
            int shift = 7-(pixel_num%8);
            int y_off = pixel_num/8;
            current_data |= ((data_begin[0x0 + (y_off*2)]&bitmask) >> shift);
            current_data |= ((data_begin[0x1 + (y_off*2)]&bitmask) >> shift) << 1;
            if (m_BitDepth == 4) {
                current_data |= ((data_begin[0x10+ (y_off*2)]&bitmask) >> shift) << 2;
                current_data |= ((data_begin[0x11+ (y_off*2)]&bitmask) >> shift) << 3;
            }
            UndoStack::UndoEntry u = {
                undo_stack.top().data_type,
                undo_stack.top().change_index,
                std::vector<uchar>({pixel_num, current_data})
            };
            redo_stack.push(u);

            uchar value = undo_stack.top().data[1];
            if((value&0b0001)!= 0)
            {
                data_begin[0x0 + (y_off*2)] |= (bitmask);
            }
            else
            {
                data_begin[0x0 + (y_off*2)] &= ~(bitmask);
            }
            if((value&0b0010)!= 0)
            {
                data_begin[0x1 + (y_off*2)] |= (bitmask);
            }
            else
            {
                data_begin[0x1 + (y_off*2)] &= ~(bitmask);
            }
            if (m_BitDepth == 4) {
                if((value&0b0100)!= 0)
                {
                    data_begin[0x10+ (y_off*2)] |= (bitmask);
                }
                else
                {
                    data_begin[0x10+ (y_off*2)] &= ~(bitmask);
                }
                if((value&0b1000)!= 0)
                {
                    data_begin[0x11+ (y_off*2)] |= (bitmask);
                }
                else
                {
                    data_begin[0x11+ (y_off*2)] &= ~(bitmask);
                }
            }
        }
        undo_stack.pop();
    }
}

void BGTileSet::redoLastEdit()
{
    if(!redo_stack.empty())
    {
        if((redo_stack.top().data_type & UndoStack::TILESET_EDIT) != 0)
        {
            uint location = redo_stack.top().change_index;
            UndoStack::UndoEntry u = {
                redo_stack.top().data_type,
                redo_stack.top().change_index,
                std::vector<uchar>(TileSet.begin()+location, TileSet.begin()+location+TILESET_ENTRY_SIZE)
            };
            undo_stack.push(u);
            std::copy(redo_stack.top().data.begin(), redo_stack.top().data.end(), TileSet.begin()+location);
        }
        else if((redo_stack.top().data_type & UndoStack::TILE_EDIT) != 0)
        {
             uint location = (redo_stack.top().change_index * m_BGTileRowSize);
             std::vector<uchar>::iterator data_begin;
             data_begin = BGTileData.begin() + location;
             UndoStack::UndoEntry u = {redo_stack.top().data_type, redo_stack.top().change_index, std::vector<uchar>(data_begin, data_begin+m_BGTileRowSize) };
             undo_stack.push(u);
             std::copy(redo_stack.top().data.begin(), redo_stack.top().data.end(), data_begin);
        }
        else if((redo_stack.top().data_type & UndoStack::PIXEL_EDIT) != 0)
        {
            uint location = (redo_stack.top().change_index * m_BGTileRowSize);
            std::vector<uchar>::iterator data_begin;
            data_begin = BGTileData.begin() + location;
            uchar pixel_num = redo_stack.top().data[0];
            uchar current_data = 0;
            int bitmask = 0x80 >> (pixel_num%8);
            int shift = 7-(pixel_num%8);
            int y_off = pixel_num/8;
            current_data |= ((data_begin[0x0 + (y_off*2)]&bitmask) >> shift);
            current_data |= ((data_begin[0x1 + (y_off*2)]&bitmask) >> shift) << 1;
            if (m_BitDepth == 4) {
                current_data |= ((data_begin[0x10+ (y_off*2)]&bitmask) >> shift) << 2;
                current_data |= ((data_begin[0x11+ (y_off*2)]&bitmask) >> shift) << 3;
            }
            UndoStack::UndoEntry u = {redo_stack.top().data_type, redo_stack.top().change_index, std::vector<uchar>({pixel_num, current_data}) };
            undo_stack.push(u);

            uchar value = redo_stack.top().data[1];
            if((value&0b0001)!= 0)
            {
                data_begin[0x0 + (y_off*2)] |= (bitmask);
            }
            else
            {
                data_begin[0x0 + (y_off*2)] &= ~(bitmask);
            }
            if((value&0b0010)!= 0)
            {
                data_begin[0x1 + (y_off*2)] |= (bitmask);
            }
            else
            {
                data_begin[0x1 + (y_off*2)] &= ~(bitmask);
            }
            if (m_BitDepth == 4) {
                if((value&0b0100)!= 0)
                {
                    data_begin[0x10+ (y_off*2)] |= (bitmask);
                }
                else
                {
                    data_begin[0x10+ (y_off*2)] &= ~(bitmask);
                }
                if((value&0b1000)!= 0)
                {
                    data_begin[0x11+ (y_off*2)] |= (bitmask);
                }
                else
                {
                    data_begin[0x11+ (y_off*2)] &= ~(bitmask);
                }
            }
        }
        redo_stack.pop();
    }
}

void BGTileSet::exportData(int mode, uchar *&dest)
{
    if (mode == TILESET_MODE ) {
        std::copy(TileSet.begin(), TileSet.end(), dest);
    } else if (mode == TILE_MODE) {
        std::copy(BGTileData.begin(), BGTileData.end(), dest);
    } else if (mode == PALETTE_MODE) {
        int pal_num = 0;
        for (auto& palette: palettes)
        {
            int pal_index = (pal_num++)*palette.size()*2;
            int color_num = 0;
            for (auto& rgb888olor: palette)
            {
                int red, blue, green;
                rgb888olor.getRgb(&red, &green, &blue);
                ushort color = ((blue<<7)&0x7C00) | ((green << 2)&0x3E0) | ((red>>3)&0x1F);
                dest[pal_index + color_num] = color&0xFF;
                dest[pal_index + color_num + 1] = color>>8;
                color_num+=2;
            }
        }
    }
}
