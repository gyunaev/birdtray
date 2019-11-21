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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

// A button which changes its background to currently selected color,
// and lets the user to select a color on click.
class ColorButton : public QPushButton
{
    Q_OBJECT

    public:
        /**
         * A button that lets the user select a color.
         * @param parent The parent widget.
         * @param color The initial selected color.
         */
        explicit ColorButton(QWidget* parent, QColor color = Qt::black);

        void	setColor( const QColor& color );
        void    allowSetAlpha( bool allow );
        
        /**
         * Enable or disable drawing the button borders.
         * @param enable Whether or not to draw the borders.
         */
        void    setBorderlessMode(bool enable);
        QColor	color() const;

    Q_SIGNALS:
        /**
         * Called when the chosen color changed.
         * @param color The new color.
         */
        void onColorChanged(const QColor &color);
        
    protected:
        void	paintEvent( QPaintEvent * event ) override;

    private slots:
        void	btnClicked();

    private:
        QColor	m_selectedColor;
        bool    m_allowSetAlpha;
        
        /**
         * Indicated whether or not to draw the button borders.
         */
        bool    drawBorder;
};

#endif // COLORBUTTON_H
