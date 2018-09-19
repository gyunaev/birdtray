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

ColorButton::ColorButton( QWidget * parent )
    : QPushButton( parent ), m_selectedColor( Qt::black )
{
    m_allowSetAlpha = false;
	connect( this, SIGNAL(clicked()), this, SLOT(btnClicked()) );
}

void ColorButton::setColor( const QColor& color )
{
	// After the constructor is called, UIC-generated code calls setLabel, so we overwrite it here
	setText( tr("") );

	m_selectedColor = color;
    update();
}

void ColorButton::allowSetAlpha(bool allow)
{
    m_allowSetAlpha = allow;
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
	QPushButton::paintEvent( event );

	// Paint a rectangle
	QPainter painter (this);

	// Take 50% of height and 80% of width
	int rectwidth = width() * 0.8;
	int rectheight = height() * 0.5;

	QRect rect( (width() - rectwidth) / 2, (height() - rectheight) / 2, rectwidth, rectheight );

	if ( isDown() )
		rect.translate( 1, 1 );

	painter.fillRect( rect, m_selectedColor );
	painter.end();
}
