/*Original work Copyright (c) 2014, delmottea
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
*  and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Modified work Copyright (c) 2018 Peter Kondratyuk. All Rights Reserved.
*
* MIT license:
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
* to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions
* of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#pragma once
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QLayout>

#include "QtUtils.h"

class CvImageWidget : public QWidget
{
	Q_OBJECT
public:
	//The parent should be a frame or a scroll area
	//CvImageVidget will set a layout to it
	CvImageWidget(QWidget *parent) : QWidget(parent)
	{
		QVBoxLayout* vLayout = new QVBoxLayout();
		parent->setLayout(vLayout);
		vLayout->setMargin(0);
		QHBoxLayout* hLayout = new QHBoxLayout();
		hLayout->setMargin(0);
		vLayout->addLayout(hLayout);
		hLayout->addWidget(this);

		hLayout->activate();
	}

	QSize sizeHint() const { return qImage.size(); }
	QSize minimumSizeHint() const { return qImage.size(); }

	int AvailableWidth() const { return parentWidget()->width() - 3; }
	int AvailableHeight() const { return parentWidget()->height() - 3; }
	QSize AvailableSize() const { return QSize(AvailableWidth(), AvailableHeight()); }

	void wheelEvent(QWheelEvent* event)
	{
		double y = event->angleDelta().y() / 8.0;	//In degrees

		emit WheelTurned(y);
	}

	void mouseMoveEvent(QMouseEvent* event)
	{
		emit MouseMoved(cv::Point2d(event->x(), event->y()));
	}

	void mousePressEvent(QMouseEvent* event)
	{
		cv::Point2d pos = cv::Point2d(event->x(), event->y());

		if (event->button() == Qt::LeftButton) emit MouseLeftPressed(pos);

		else if (event->button() == Qt::RightButton) emit MouseRightPressed(pos);

		else if (event->button() == Qt::MidButton) emit MouseMiddlePressed(pos);
	}

	void mouseReleaseEvent(QMouseEvent* event)
	{
		cv::Point2d pos = cv::Point2d(event->x(), event->y());

		if (event->button() == Qt::LeftButton) emit MouseLeftReleased(pos);

		else if (event->button() == Qt::RightButton) emit MouseRightReleased(pos);

		else if (event->button() == Qt::MidButton) emit MouseMiddleReleased(pos);
	}


signals:
	//The user turned mouse wheel by delta degrees
	void WheelTurned(double delta);
	//The user presses and releases mouse buttons
	void MouseLeftPressed(cv::Point2d pos);
	void MouseRightPressed(cv::Point2d pos);
	void MouseMiddlePressed(cv::Point2d pos);
	void MouseLeftReleased(cv::Point2d pos);
	void MouseRightReleased(cv::Point2d pos);
	void MouseMiddleReleased(cv::Point2d pos);
	//The user moves the mouse while one of the buttons is pressed
	void MouseMoved(cv::Point2d pos);


public slots:
	void ShowImage(const cv::Mat& image)
	{
		if (image.type() == CV_8UC3) cvtColor(image, mat, CV_BGR2RGB);
		else if (image.type() == CV_8UC1) cvtColor(image, mat, CV_GRAY2RGB);

		qImage = QImage(mat.data, mat.cols, mat.rows, mat.cols * 3, QImage::Format_RGB888);
		setFixedSize(image.cols, image.rows);

		repaint();
	}

	//Uses parent dimensions, but preserves image proportions
	//Uses gray color to fill unused areas
	void RescaleToParentAndShow(const cv::Mat& image)
	{
		RescaleAndShow(image, AvailableWidth(), AvailableHeight());
	}

	//Tries to preserve image proportions
	void RescaleAndShow(const cv::Mat& image, int width, int height)
	{
		if (width == 0 || height == 0) return;

		int newImWidth, newImHeight;
		double rescaleFactor;

		//Figure out whether the image is relatively wider than the given size
		double ratio = double(image.cols) / double(image.rows) / (double(width) / double(height));

		//Yes, the image is relatively wider
		//The width of the size given will determine the image size
		if (ratio >= 1)
		{
			newImWidth = width;
			rescaleFactor = double(newImWidth) / double(image.cols);
			if (ratio == 1) newImHeight = height;
			else newImHeight = floor(double(image.rows) * rescaleFactor);
		}
		else  //The height of the size given will determine the image size	
		{
			newImHeight = height;
			rescaleFactor = double(newImHeight) / double(image.rows);
			newImWidth = floor(double(image.cols) * rescaleFactor);
		}

		int interpType;
		if (rescaleFactor >= 1) interpType = cv::INTER_LINEAR;	//If we are enlarging, linear interpolation
		else interpType = cv::INTER_AREA;						//If shrinking, area summation

		cv::Mat rescImage;
		if (rescaleFactor == 1) image.copyTo(rescImage);
		else cv::resize(image,rescImage,cv::Size(newImWidth, newImHeight),0,0,interpType);

		ShowImage(rescImage);
	}

	void Clear() { ShowImage(cv::Mat(0, 0, CV_8UC3)); }

protected:
	void paintEvent(QPaintEvent*)
	{
		QPainter painter(this);
		painter.drawImage(QPoint(0, 0), qImage);
		painter.end();
	}

	cv::Mat mat;
	QImage qImage;
};