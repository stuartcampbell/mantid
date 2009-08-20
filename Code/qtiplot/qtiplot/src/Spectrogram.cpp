/***************************************************************************
	File                 : Spectrogram.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot's Spectrogram Class
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "Spectrogram.h"
#include <math.h>
#include <QPen>
#include <qwt_scale_widget.h>
#include <QColor>
#include <qwt_painter.h>
#include "qwt_scale_engine.h"
#include <QPainter>
#include <qwt_symbol.h>


#include <iostream>
#include "MantidAPI/MatrixWorkspace.h"
using namespace Mantid::API;

//Mantid::Kernel::Logger & Spectrogram::g_log=Mantid::Kernel::Logger::get("Spectrogram");

Spectrogram::Spectrogram():
	QwtPlotSpectrogram(),
	d_matrix(0),d_funct(0),//Mantid
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),
	color_map(QwtLinearColorMap())
{
}

Spectrogram::Spectrogram(Matrix *m):
	QwtPlotSpectrogram(QString(m->objectName())),
	d_matrix(m),d_funct(0),//Mantid
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),mColorMap()
	//color_map(QwtLinearColorMap()),
{
setData(MatrixData(m));
double step = fabs(data().range().maxValue() - data().range().minValue())/5.0;

QwtValueList contourLevels;
for ( double level = data().range().minValue() + step;
	level < data().range().maxValue(); level += step )
    contourLevels += level;

setContourLevels(contourLevels);
}

Spectrogram::Spectrogram(UserHelperFunction *f,int nrows, int ncols,double left, double top, double width, double height,double minz,double maxz)
:	QwtPlotSpectrogram(),
	d_matrix(0),d_funct(f),
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),
	color_map(QwtLinearColorMap())
{
    setData(FunctionData(f,nrows,ncols,left,top,width,height,minz,maxz));
    double step = fabs(data().range().maxValue() - data().range().minValue())/5.0;

    QwtValueList contourLevels;
    for ( double level = data().range().minValue() + step;
	    level < data().range().maxValue(); level += step )
        contourLevels += level;

    setContourLevels(contourLevels);
	
}

Spectrogram::Spectrogram(UserHelperFunction *f,int nrows, int ncols,QwtDoubleRect bRect,double minz,double maxz,Graph *graph)
:	QwtPlotSpectrogram(),d_graph(graph),
d_matrix(0),d_funct(f),
color_axis(QwtPlot::yRight),
color_map_policy(Default),
d_show_labels(false),
d_labels_color(Qt::black),
d_labels_font(QFont()),
d_white_out_labels(false),
d_labels_angle(0.0),
d_labels_x_offset(0),
d_labels_y_offset(0),
d_selected_label(NULL),
d_labels_align(Qt::AlignHCenter),
m_ScaleType(1),
//mWorkspaceSptr(workspace),
mColorMap(), mDataMinValue(minz), mDataMaxValue(maxz), 
mBinMinValue(DBL_MAX), mBinMaxValue(-DBL_MAX), mWkspDataMin(DBL_MAX), mWkspDataMax(-DBL_MAX), 
mWkspBinMin(DBL_MAX), mWkspBinMax(-DBL_MAX),m_nRows(nrows),m_nColumns(ncols),mScaledValues(0)
{
	setTitle("UserHelperFunction");
	setData(FunctionData(f,nrows,ncols,bRect,minz,maxz));
	double step = fabs(data().range().maxValue() - data().range().minValue())/5.0;

	QwtValueList contourLevels;
	for ( double level = data().range().minValue() + step;
		level < data().range().maxValue(); level += step )
		contourLevels += level;
 	setContourLevels(contourLevels);
	//loadSettings();

}
Spectrogram::~Spectrogram()
{
	//saveSettings();
}
void Spectrogram::setContourLevels (const QwtValueList & levels)
{
	QwtPlotSpectrogram::setContourLevels(levels);
	createLabels();
}

void Spectrogram::updateData(Matrix *m)
{
if (!m)
	return;

QwtPlot *plot = this->plot();
if (!plot)
	return;

setData(MatrixData(m));
setLevelsNumber(levels());

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
if (colorAxis)
{
	colorAxis->setColorMap(data().range(), colorMap());
}
plot->setAxisScale(color_axis, data().range().minValue(), data().range().maxValue());
plot->replot();
}

void Spectrogram::setLevelsNumber(int levels)
{
double step = fabs(data().range().maxValue() - data().range().minValue())/(double)levels;

QwtValueList contourLevels;
for ( double level = data().range().minValue() + step;
	level < data().range().maxValue(); level += step )
    contourLevels += level;

setContourLevels(contourLevels);
}

bool Spectrogram::hasColorScale()
{
	QwtPlot *plot = this->plot();
	if (!plot)
		return false;

	if (!plot->axisEnabled (color_axis))
		return false;

	QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
	return colorAxis->isColorBarEnabled();
}

void Spectrogram::showColorScale(int axis, bool on)
{
if (hasColorScale() == on && color_axis == axis)
	return;

QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
colorAxis->setColorBarEnabled(false);

color_axis = axis;

// We must switch main and the color scale axes and their respective scales
int xAxis = this->xAxis();
int yAxis = this->yAxis();
int oldMainAxis;
if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
	{
	oldMainAxis = xAxis;
	xAxis = 5 - color_axis;
	}
else if (axis == QwtPlot::yLeft || axis == QwtPlot::yRight)
	{
	oldMainAxis = yAxis;
	yAxis = 1 - color_axis;
	}

// First we switch axes
setAxis(xAxis, yAxis);

// Next we switch axes scales
QwtScaleDiv *scDiv = plot->axisScaleDiv(oldMainAxis);
if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
	plot->setAxisScale(xAxis, scDiv->lBound(), scDiv->hBound());
else if (axis == QwtPlot::yLeft || color_axis == QwtPlot::yRight)
	plot->setAxisScale(yAxis, scDiv->lBound(), scDiv->hBound());

colorAxis = plot->axisWidget(color_axis);
plot->setAxisScale(color_axis, data().range().minValue(), data().range().maxValue());
colorAxis->setColorBarEnabled(on);
colorAxis->setColorMap(data().range(), colorMap());
if (!plot->axisEnabled(color_axis))
	plot->enableAxis(color_axis);
colorAxis->show();
plot->updateLayout();
}

int Spectrogram::colorBarWidth()
{
QwtPlot *plot = this->plot();
if (!plot)
	return 0;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
return colorAxis->colorBarWidth();
}

void Spectrogram::setColorBarWidth(int width)
{
QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
colorAxis->setColorBarWidth(width);
}

Spectrogram* Spectrogram::copy()
{
Spectrogram *new_s = new Spectrogram(matrix());
new_s->setDisplayMode(QwtPlotSpectrogram::ImageMode, testDisplayMode(QwtPlotSpectrogram::ImageMode));
new_s->setDisplayMode(QwtPlotSpectrogram::ContourMode, testDisplayMode(QwtPlotSpectrogram::ContourMode));
new_s->setColorMap (colorMap());
new_s->setAxis(xAxis(), yAxis());
new_s->setDefaultContourPen(defaultContourPen());
new_s->setLevelsNumber(levels());
new_s->color_map_policy = color_map_policy;
return new_s;
}

void Spectrogram::setGrayScale()
{
color_map = QwtLinearColorMap(Qt::black, Qt::white);
setColorMap(color_map);
//setColorMap(mColorMap);
color_map_policy = GrayScale;

QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
if (colorAxis)
{
	colorAxis->setColorMap(data().range(), colorMap());
}
}

void Spectrogram::setDefaultColorMap()
{
	color_map = defaultColorMap();
	//setColorMap(color_map);
	setColorMap(mColorMap);
	color_map_policy = Default;

	QwtPlot *plot = this->plot();
	if (!plot)
		return;

	QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
	if (colorAxis)
			colorAxis->setColorMap(this->data().range(), this->colorMap());
	
}
void Spectrogram::setCustomColorMap(const QwtColorMap &map)
{
	setColorMap(map);
	//color_map = map;
	color_map_policy = Custom;

	QwtPlot *plot = this->plot();
	if (!plot)
		return;

	QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
	if (colorAxis)
	{
		colorAxis->setColorMap(this->data().range(), this->colorMap());
	}
}
void Spectrogram::setCustomColorMap(const QwtLinearColorMap& map)
{
setColorMap(map);
color_map = map;
color_map_policy = Custom;

QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
if (colorAxis)
{
	colorAxis->setColorMap(this->data().range(), this->colorMap());
}
}

QwtLinearColorMap Spectrogram::defaultColorMap()
{
QwtLinearColorMap colorMap(Qt::blue, Qt::red);
colorMap.addColorStop(0.25, Qt::cyan);
colorMap.addColorStop(0.5, Qt::green);
colorMap.addColorStop(0.75, Qt::yellow);
return colorMap;
}
void Spectrogram::setColorMapPen(bool on)
{
	if (d_color_map_pen == on)
		return;

	d_color_map_pen = on;
	if (on) {
		setDefaultContourPen(Qt::NoPen);
		d_pen_list.clear();
	}
}
/**
for creating contour line labels
*/
void Spectrogram::createLabels()
{
	foreach(QwtPlotMarker *m, d_labels_list){
		m->detach();
		delete m;
	}
	d_labels_list.clear();
	
	int index = 0;
	QwtValueList levels = contourLevels();
	const int numLevels = levels.size();
    for (int l = 0; l < numLevels; l++){
		PlotMarker *m = new PlotMarker(l, d_labels_angle);
		QwtText t = QwtText(QString::number(levels[l]));
		t.setColor(d_labels_color);
		t.setFont(d_labels_font);

		if (d_white_out_labels)
			t.setBackgroundBrush(QBrush(Qt::white));
        else
            t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);

        int x_axis = xAxis();
        int y_axis = yAxis();
		m->setAxis(x_axis, y_axis);
		
		//QwtPlot *d_plot = plot();
		//if (!d_plot)
		//	return;
	
		//QSize size = t.textSize();
  //      int dx = int(d_labels_x_offset*0.01*size.height());
  //      int dy = -int((d_labels_y_offset*0.01 + 0.5)*size.height());
  //      int x2 =80;//d_plot->transform(x_axis, levels[l]) + dx;
  //      int y2 =80;//d_plot->transform(y_axis, levels[l]) + dy;
  //      switch(d_labels_align){
  //          case Qt::AlignLeft:
  //          break;
  //          case Qt::AlignHCenter:
  //              x2 -= size.width()/2;
  //          break;
  //          case Qt::AlignRight:
  //              x2 -= size.width();
  //          break;
  //      }
  //      m->setXValue(d_plot->invTransform(x_axis, x2));
  //      m->setYValue(d_plot->invTransform(y_axis, y2));
	

  //      if (d_graph && d_show_labels)
		//{//g_log.error()<<"plotmarker attach called"<<std::endl;
		//	m->attach(d_plot);
		//}
		d_labels_list << m;
		index++;
	}
}
void Spectrogram::showContourLineLabels(bool show)
{
	d_show_labels=false;
	if (show == d_show_labels)
	{
        return;
	}
    d_show_labels = show;
	createLabels();
	QwtPlot *d_plot = plot();
		if (!d_plot)
			return;
	foreach(PlotMarker *m, d_labels_list){
		if (d_show_labels)
		{		m->attach(d_plot);
		}
		else
			m->detach();
	}
}
bool Spectrogram::hasSelectedLabels()
{
    /*if (d_labels_list.isEmpty())
        return false;

    foreach(PlotMarker *m, d_labels_list){
        if (m->label().backgroundPen() == QPen(Qt::blue))
            return true;
        else
            return false;
    }
    return false;*/

    if (d_selected_label)
		return true;
	return false;
}
void Spectrogram::selectLabel(bool on)
{
	if (on){
		d_graph->deselect();
		d_graph->notifyFontChange(d_labels_font);
	}

	foreach(PlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if(t.text().isEmpty())
			return;

		if (d_selected_label && m == d_selected_label && on)
			t.setBackgroundPen(QPen(Qt::blue));
		else
			t.setBackgroundPen(QPen(Qt::NoPen));

		m->setLabel(t);
	}

	d_graph->replot();
}


bool Spectrogram::selectedLabels(const QPoint& pos)
{
	d_selected_label = NULL;

	if (d_graph->hasActiveTool())
		return false;
    /*foreach(PlotMarker *m, d_labels_list){
        int x = d_graph->transform(xAxis(), m->xValue());
        int y = d_graph->transform(yAxis(), m->yValue());

        QMatrix wm;
        wm.translate(x, y);
		wm.rotate(-d_labels_angle);
        if (wm.mapToPolygon(QRect(QPoint(0, 0), m->label().textSize())).containsPoint(pos, Qt::OddEvenFill)){
			d_selected_label = m;
			d_click_pos_x = d_graph->invTransform(xAxis(), pos.x());
			d_click_pos_y = d_graph->invTransform(yAxis(), pos.y());
            selectLabel(true);
            return true;
        }
	}*/
	return false;
}

QString Spectrogram::saveToString()
{
QString s = "<spectrogram>\n";
s += "\t<matrix>" + QString(d_matrix->objectName()) + "</matrix>\n";

if (color_map_policy != Custom)
	s += "\t<ColorPolicy>" + QString::number(color_map_policy) + "</ColorPolicy>\n";
else
	{
	s += "\t<ColorMap>\n";
	s += "\t\t<Mode>" + QString::number(color_map.mode()) + "</Mode>\n";
	s += "\t\t<MinColor>" + color_map.color1().name() + "</MinColor>\n";
	s += "\t\t<MaxColor>" + color_map.color2().name() + "</MaxColor>\n";
	QwtArray <double> colors = color_map.colorStops();
	int stops = (int)colors.size();
	s += "\t\t<ColorStops>" + QString::number(stops - 2) + "</ColorStops>\n";
	for (int i = 1; i < stops - 1; i++)
		{
		s += "\t\t<Stop>" + QString::number(colors[i]) + "\t";
		s += QColor(color_map.rgb(QwtDoubleInterval(0,1), colors[i])).name();
		s += "</Stop>\n";
		}
	s += "\t</ColorMap>\n";
	}
s += "\t<Image>"+QString::number(testDisplayMode(QwtPlotSpectrogram::ImageMode))+"</Image>\n";

bool contourLines = testDisplayMode(QwtPlotSpectrogram::ContourMode);
s += "\t<ContourLines>"+QString::number(contourLines)+"</ContourLines>\n";
if (contourLines)
	{
	s += "\t\t<Levels>"+QString::number(levels())+"</Levels>\n";
	bool defaultPen = defaultContourPen().style() != Qt::NoPen;
	s += "\t\t<DefaultPen>"+QString::number(defaultPen)+"</DefaultPen>\n";
	if (defaultPen)
		{
		s += "\t\t\t<PenColor>"+defaultContourPen().color().name()+"</PenColor>\n";
		s += "\t\t\t<PenWidth>"+QString::number(defaultContourPen().widthF())+"</PenWidth>\n";
		s += "\t\t\t<PenStyle>"+QString::number(defaultContourPen().style() - 1)+"</PenStyle>\n";
		}
	}
QwtScaleWidget *colorAxis = plot()->axisWidget(color_axis);
if (colorAxis && colorAxis->isColorBarEnabled())
	{
	s += "\t<ColorBar>\n\t\t<axis>" + QString::number(color_axis) + "</axis>\n";
	s += "\t\t<width>" + QString::number(colorAxis->colorBarWidth()) + "</width>\n";
	s += "\t</ColorBar>\n";
	}
s += "\t<Visible>"+ QString::number(isVisible()) + "</Visible>\n";
return s+"</spectrogram>\n";
}
/**
 * Returns a reference to the constant colormap
 */
const MantidColorMap & Spectrogram::getColorMap() const
{
  return mColorMap;
}
void Spectrogram::setMantidColorMap(const MantidColorMap &map)
{
	setColorMap(map);
}
/**
 * Returns a reference to the colormap
 */
MantidColorMap & Spectrogram::mutableColorMap()
{
  return mColorMap;
}
void Spectrogram::setupColorBarScaling()
{
	
  double minValue = mDataMinValue;
  double maxValue = mDataMaxValue;
  QwtScaleWidget *rightAxis = plot()->axisWidget(QwtPlot::yRight); 
  if(rightAxis==NULL) return;
  QwtScaleDiv *scDiv = plot()->axisScaleDiv(QwtPlot::yRight);
  QwtValueList lst = scDiv->ticks (QwtScaleDiv::MajorTick);
  double majTicks=lst.count();
  lst = scDiv->ticks (QwtScaleDiv::MinorTick);
  double minTicks=lst.count();
  
  int scaleType=getScaleType();
  MantidColorMap::ScaleType type =(MantidColorMap::ScaleType)scaleType;
  if( type == MantidColorMap::Linear )
  {
    QwtLinearScaleEngine linScaler;
    rightAxis->setScaleDiv(linScaler.transformation(), linScaler.divideScale(minValue, maxValue,  majTicks,minTicks));
    rightAxis->setColorMap(QwtDoubleInterval(minValue, maxValue),getColorMap());  
  }
  else
 {
    QwtLog10ScaleEngine logScaler;    
    double logmin(minValue);
    if( logmin < 1.0 )
    {
      logmin = 1.0;
    }
   // rightAxis->setScaleDiv(logScaler.transformation(), logScaler.divideScale(logmin, maxValue, 20, 5));
	rightAxis->setScaleDiv(logScaler.transformation(), logScaler.divideScale(logmin, maxValue, majTicks, minTicks));
    rightAxis->setColorMap(QwtDoubleInterval(minValue, maxValue), getColorMap());
  }
}
void Spectrogram::setScaleType(int scaleType)
{
	m_ScaleType=scaleType;
}
int Spectrogram::getScaleType()const
{
	return m_ScaleType;
}
/**
 * Save properties of the window a persistent store
 */
void Spectrogram::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Mantid/2DPlotSpectrogram");
  //settings.setValue("BackgroundColor", mInstrumentDisplay->currentBackgroundColor());
  settings.setValue("ColormapFile", mCurrentColorMap);
  settings.setValue("ScaleType",getColorMap().getScaleType());
  settings.endGroup();
}
/**
 * This method loads the setting from QSettings
 */
void Spectrogram::loadSettings()
{
  //Load Color
  QSettings settings;
  settings.beginGroup("Mantid/2DPlotSpectrogram");
  
  //Load Colormap. If the file is invalid the default stored colour map is used
  mCurrentColorMap = settings.value("ColormapFile", "").toString();
  // Set values from settings
  mutableColorMap().loadMap(mCurrentColorMap);
  
  MantidColorMap::ScaleType type = (MantidColorMap::ScaleType)settings.value("ScaleType", MantidColorMap::Log10).toUInt();
  
  mutableColorMap().changeScaleType(type);
  
  settings.endGroup();
}
/**
 * This method saves the selectcolrmap file name to membervaraible
 */
void Spectrogram::setColorMapFileName(QString colormapName)
{
	mCurrentColorMap=colormapName;
}
QwtDoubleRect Spectrogram::boundingRect() const
{
    return d_matrix?d_matrix->boundingRect() : data().boundingRect();
}
void Spectrogram::setContourPenList(QList<QPen> lst)
{
	d_pen_list = lst;
	setDefaultContourPen(Qt::NoPen);
	d_color_map_pen = false;
}

double MatrixData::value(double x, double y) const
{		
x += 0.5*dx;
y -= 0.5*dy;
	
int i = abs((y - y_start)/dy);
int j = abs((x - x_start)/dx);

if (d_m && i >= 0 && i < n_rows && j >=0 && j < n_cols)
	return d_m[i][j];
else
	return 0.0;
}
void Spectrogram::setLabelsRotation(double angle)
{
    if (angle == d_labels_angle)
        return;

    d_labels_angle = angle;

    foreach(PlotMarker *m, d_labels_list)
		m->setAngle(angle);
}

void Spectrogram::setLabelsOffset(double x, double y)
{
    if (x == d_labels_x_offset && y == d_labels_y_offset)
        return;

    d_labels_x_offset = x;
    d_labels_y_offset = y;
}

void Spectrogram::setLabelOffset(int index, double x, double y)
{
	if (index < 0 || index >= d_labels_list.size())
		return;

	PlotMarker *m = d_labels_list[index];
	if (!m)
		return;

	m->setLabelOffset(x, y);
}

void Spectrogram::setLabelsWhiteOut(bool whiteOut)
{
    if (whiteOut == d_white_out_labels)
        return;

    d_white_out_labels = whiteOut;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if (whiteOut)
			t.setBackgroundBrush(QBrush(Qt::white));
        else
            t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);
	}
}
void Spectrogram::drawContourLines (QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtRasterData::ContourLines &contourLines) const
{
	//QwtPlotSpectrogram::drawContourLines(p, xMap, yMap, contourLines);

	QwtValueList levels = contourLevels();
    const int numLevels = (int)levels.size();
    for (int l = 0; l < numLevels; l++){
        const double level = levels[l];

        QPen pen = defaultContourPen();
        if ( pen.style() == Qt::NoPen )
            pen = contourPen(level);

        if ( pen.style() == Qt::NoPen )
            continue;

       // p->setPen(QwtPainter::scaledPen(pen));

        const QPolygonF &lines = contourLines[level];
        for ( int i = 0; i < (int)lines.size(); i += 2 ){
            const QPointF p1( xMap.xTransform(lines[i].x()),
                yMap.transform(lines[i].y()) );
            const QPointF p2( xMap.xTransform(lines[i + 1].x()),
                yMap.transform(lines[i + 1].y()) );

            p->drawLine(p1, p2);
        }
    }

	if (d_show_labels)
		updateLabels(p, xMap, yMap, contourLines);
}

void Spectrogram::updateLabels(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QwtRasterData::ContourLines &contourLines) const
{
	/*QwtValueList levels = contourLevels();
	const int numLevels = levels.size();
	int x_axis = xAxis();
	int y_axis = yAxis();
    for (int l = 0; l < numLevels; l++){
        const double level = levels[l];
        const QPolygonF &lines = contourLines[level];
        int i = (int)lines.size()/2;

		PlotMarker *mrk = d_labels_list[l];
		if (!mrk)
			return;

		QSize size = mrk->label().textSize();
        int dx =int((d_labels_x_offset )*0.01*size.height()); //int((d_labels_x_offset + mrk->xLabelOffset())*0.01*size.height());
        int dy = -int(((d_labels_y_offset )*0.01 + 0.5)*size.height());

		double x = lines[i].x();
		double y = lines[i].y();
		int x2 = d_graph->transform(x_axis, x) + dx;
        int y2 = d_graph->transform(y_axis, y) + dy;

        mrk->setValue(d_graph->invTransform(x_axis, x2),
					d_graph->invTransform(y_axis, y2));
    }*/
}
/**
     for setting the lables color on contour lines
*/
void Spectrogram::setLabelsColor(const QColor& c)
{
    if (c == d_labels_color)
        return;

    d_labels_color = c;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setColor(c);
		m->setLabel(t);
	}
}
/**
 * Integrate the workspace
 */
void Spectrogram::calculateColorCounts(boost::shared_ptr<Mantid::API::MatrixWorkspace> workspace)
{
  /* std::vector<int> detector_list(0);
  mInstrumentActor->getDetectorIDList(detector_list);
  if( detector_list.empty() ) return;
  std::vector<int> index_list;
  getSpectraIndexList(detector_list, index_list);
  */
	QwtLinearColorMap color_map;
   const int n_spec=m_nRows ;//= index_list.size();
  std::vector<double> integrated_values(n_spec, 0.0);
  mWkspDataMin = DBL_MAX;
  mWkspDataMax = -DBL_MAX;
  for( int i = 0; i < n_spec; ++i )
  {
    int widx=i ;//= index_list[i];
    if( widx != -1 )
    {
      double sum = integrateSingleSpectra(workspace, widx);
      integrated_values[i] = sum;
      if( sum < mWkspDataMin )
      {
	mWkspDataMin = sum;
      }
      else if( sum > mWkspDataMax )
      {
	mWkspDataMax = sum;
      }
      else continue;
      
    }
    else
    {
      integrated_values[i] = -1.0;
    }
  }

  // No preset value
  if( std::abs(mDataMinValue - DBL_MAX)/DBL_MAX < 1e-08 )
  {
    mDataMinValue = mWkspDataMin;
  }

  if( (mDataMaxValue + DBL_MAX)/DBL_MAX < 1e-08 )
  {
    mDataMaxValue = mWkspDataMax;
  }

  const short max_ncols = mColorMap.getLargestAllowedCIndex() + 1;
  mScaledValues = std::vector<unsigned char>(n_spec, 0);
  std::vector<boost::shared_ptr<GLColor> > colorlist(n_spec);
  QwtDoubleInterval wksp_interval(mWkspDataMin, mWkspDataMax);
  QwtDoubleInterval user_interval(mDataMinValue, mDataMaxValue);

  std::vector<double>::const_iterator val_end = integrated_values.end();
  int idx(0);
  for( std::vector<double>::const_iterator val_itr = integrated_values.begin(); val_itr != val_end; 
       ++val_itr, ++idx )
  {
    unsigned char c_index(mColorMap.getTopCIndex());
    if( (*val_itr) < 0.0 ) 
    {
      mScaledValues[idx] = mColorMap.getLargestAllowedCIndex();
    }
    else
    {
      // Index to store
      short index = std::floor( mColorMap.normalize(user_interval, *val_itr)*max_ncols );
      if( index >= max_ncols )
      {
	index = max_ncols;
      }
      else if( index < 0 )
      {
	index = 0;
      }
      else {}
      mScaledValues[idx] = static_cast<unsigned char>(index);
      c_index = mColorMap.colorIndex(user_interval, *val_itr);

    }
    colorlist[idx] = mColorMap.getColor(c_index);
	QRgb qrgb=mColorMap.rgb(user_interval, *val_itr);
	QColor qclr(qrgb);
	//GLColor clr;
	color_map.addColorStop(idx, qclr);
  }
   QwtColorMap& qwtMap = dynamic_cast<QwtLinearColorMap&>( color_map);//QwtLinearColorMap
  // mColorMap=qwtMap;
  // setCustomColorMap(qwtMap);
   setColorMap(qwtMap);
  }


double Spectrogram::integrateSingleSpectra(boost::shared_ptr<Mantid::API::MatrixWorkspace> workspace, const int wksp_index)
{
  std::vector<double>::const_iterator bin_itr = workspace->readX(wksp_index).begin();
  std::vector<double>::const_iterator bin_end = workspace->readX(wksp_index).end();
  std::vector<double>::const_iterator data_end = workspace->readY(wksp_index).end();
  double sum(0.0);
  for( std::vector<double>::const_iterator data_itr = workspace->readY(wksp_index).begin();
       data_itr != data_end; ++data_itr, ++ bin_itr )
  {
    double binvalue = *bin_itr;
    if( binvalue >= mBinMinValue && binvalue <= mBinMaxValue )
    {
      sum += *data_itr;
    }
  }
  return sum;
}
/**
 * Update the colors based on a change in the maximum data value
 */
void Spectrogram::updateForNewMaxData(const double new_max)
{
  // If the new value is the same
  if( std::abs(new_max - mDataMaxValue) / mDataMaxValue < 1e-08 ) 
  {
	  return;
  }
  mDataMaxValue = new_max;
 // recount();
}

/**
 * Update the colors based on a change in the maximum data value
 */
void Spectrogram::updateForNewMinData(const double new_min)
{
  // If the new value is the same
  if( std::abs(new_min - mDataMinValue) / mDataMinValue < 1e-08 ) 
  {	  return;
  }
  mDataMinValue = new_min;
  //recount();
}
/**
 * Run a recount for the current workspace
 */
void Spectrogram::recount()
{
  // calculateColorCounts(mWorkspaceSptr);
 }




