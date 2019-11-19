/**************************************************************************
 *  Spivak Karaoke PLayer - a free, cross-platform desktop karaoke player *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include <QColorDialog>
#include <QPainter>

#include "colorbutton.h"

ColorButton::ColorButton(QWidget * parent, QColor color)
    : QPushButton(parent), m_selectedColor(std::move(color))
{
    m_allowSetAlpha = false;
    drawBorder = true;
    connect( this, SIGNAL(clicked()), this, SLOT(btnClicked()) );
}

void ColorButton::setColor( const QColor& color )
{
    // After the constructor is called, UIC-generated code calls setLabel, so we overwrite it here
    setText("");

    m_selectedColor = color;
    update();
    emit onColorChanged(m_selectedColor);
}

void ColorButton::allowSetAlpha(bool allow)
{
    m_allowSetAlpha = allow;
}

void ColorButton::setBorderlessMode(bool enable) {
    drawBorder = !enable;
}

QColor ColorButton::color() const
{
    return m_selectedColor;
}

void ColorButton::btnClicked()
{
    QColor newcolor = QColorDialog::getColor( color(),
                                              this,
                                              text(),
                                              m_allowSetAlpha ?
                                                  QColorDialog::ShowAlphaChannel :
                                                  QColorDialog::ColorDialogOptions() );

    if ( newcolor.isValid() )
        setColor( newcolor );
}

void ColorButton::paintEvent( QPaintEvent * event )
{
    if (drawBorder) {
        QPushButton::paintEvent(event);
    }
    
    // Paint a rectangle
    QPainter painter(this);
    QRect rect;
    if (drawBorder) {
        // Take 50% of height and 80% of width
        int width = qRound(this->width() * 0.8);
        int height = qRound(this->height() * 0.5);
        rect.setRect((this->width() - width) / 2, (this->height() - height) / 2, width, height);
    } else {
        rect.setRect(1, 1, width() - 2, height() - 2);
    }

    if ( isDown() )
        rect.translate( 1, 1 );

    painter.fillRect( rect, m_selectedColor );
    painter.end();
}
