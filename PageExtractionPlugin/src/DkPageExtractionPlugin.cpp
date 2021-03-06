/*******************************************************************************************************
 DkPageExtractionPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Markus Diem

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "DkPageExtractionPlugin.h"
#include "DkPageSegmentation.h"

#include "DkImageStorage.h"
#include "DkMetaData.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QDebug>
#pragma warning(pop)		// no warnings from includes - end

namespace nmp {

/**
*	Constructor
**/
DkPageExtractionPlugin::DkPageExtractionPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_crop_to_page] = "1638a7f56b814ee48c6eb8a7710e74b4";
	runIds[id_crop_to_metadata] = "51fa39637199421da680699817ac2b46";
	runIds[id_draw_to_page] = "2af5c9f018ce4a0fbfaacc5e3a48a4b5";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);
		
	menuNames[id_crop_to_page] = tr("Crop to Page");
	menuNames[id_crop_to_metadata] = tr("Crop to Metadata");
	menuNames[id_draw_to_page] = tr("Draw to Page");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_crop_to_page] = tr("Finds a page in a document image and then crops the image to that page.");
	statusTips[id_crop_to_metadata] = tr("Finds a page in a document image and then saves the coordinates to the XMP metadata.");
	statusTips[id_draw_to_page] = tr("Finds a page in a document image and then draws the found document boundaries.");
	mMenuStatusTips = statusTips.toList();
}

/**
*	Destructor
**/
DkPageExtractionPlugin::~DkPageExtractionPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString DkPageExtractionPlugin::id() const {

	return PLUGIN_ID;
};


/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkPageExtractionPlugin::image() const {

	return QImage(":/PageExtractionPlugin/img/page-extraction.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkPageExtractionPlugin::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> DkPageExtractionPlugin::createActions(QWidget* parent) {

	if (mActions.empty()) {
		QAction* ca = new QAction(mMenuNames[id_crop_to_page], this);
		ca->setObjectName(mMenuNames[id_crop_to_page]);
		ca->setStatusTip(mMenuStatusTips[id_crop_to_page]);
		ca->setData(mRunIDs[id_crop_to_page]);	// runID needed for calling function runPlugin()
		mActions.append(ca);

		ca = new QAction(mMenuNames[id_crop_to_metadata], this);
		ca->setObjectName(mMenuNames[id_crop_to_metadata]);
		ca->setStatusTip(mMenuStatusTips[id_crop_to_metadata]);
		ca->setData(mRunIDs[id_crop_to_metadata]);	// runID needed for calling function runPlugin()
		mActions.append(ca);

		ca = new QAction(mMenuNames[id_draw_to_page], this);
		ca->setObjectName(mMenuNames[id_draw_to_page]);
		ca->setStatusTip(mMenuStatusTips[id_draw_to_page]);
		ca->setData(mRunIDs[id_draw_to_page]);	// runID needed for calling function runPlugin()
		mActions.append(ca);
	}

	return mActions;
}

QList<QAction*> DkPageExtractionPlugin::pluginActions() const {

	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DkPageExtractionPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!mRunIDs.contains(runID) || !imgC)
		return imgC;

	cv::Mat img = nmc::DkImage::qImage2Mat(imgC->image());

	// run the page segmentation
	DkPageSegmentation segM(img);
	segM.compute();
	segM.filterDuplicates();

	// crop image
	if(runID == mRunIDs[id_crop_to_page]) {
		imgC->setImage(segM.getCropped(imgC->image()), tr("Page Cropped"));
	}
	// save to metadata
	if(runID == mRunIDs[id_crop_to_metadata]) {
		
		if (segM.getRects().empty())
			imgC = QSharedPointer<nmc::DkImageContainer>();	// notify parent
		else {
			nmc::DkRotatingRect rect = segM.getMaxRect().toRotatingRect();
			
			QSharedPointer<nmc::DkMetaDataT> m = imgC->getMetaData();
			m->saveRectToXMP(rect, imgC->image().size());
		}
	}
	// draw rectangles to the image
	else if(runID == mRunIDs[id_draw_to_page]) {
		
		QImage dImg = imgC->image();
		segM.draw(dImg);
		imgC->setImage(dImg, tr("Page Annotated"));
	}

	// wrong runID? - do nothing
	return imgC;
};

};

