import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import QMLOpenGL 1.0

Pixpad {
	id: glview
	width: 800
	height: 600
	anchors.fill: parent

	Component.onCompleted: {
		console.log("Component.onCompleted")
		glview.vert0 = Qt.point(pt0.x + pt0.width * 0.5, pt0.y + pt0.height * 0.5)
		glview.vert1 = Qt.point(pt1.x + pt1.width * 0.5, pt1.y + pt1.height * 0.5)
		glview.vert2 = Qt.point(pt2.x + pt2.width * 0.5, pt2.y + pt2.height * 0.5)
	}

 Image {
	id: pt0
	x: 384
	y: 138
	width: 32
	height: 32
	source: "res/button.png"
	opacity: 0.1
	onXChanged: glview.vert0 = Qt.point(this.x + this.width * 0.5, this.y + this.height * 0.5)
	onYChanged: glview.vert0 = Qt.point(this.x + this.width * 0.5, this.y + this.height * 0.5)

	MouseArea {
		id: mouse0
		anchors.fill: parent
		drag.target: pt0
		drag.axis: Drag.XAndYAxis
		drag.minimumX: 0
		drag.minimumY: 0
		drag.maximumX: glview.width - pt0.width
		drag.maximumY: glview.height - pt0.height
		onPressed: {
			pt0.opacity = 0.8
		}
		onReleased: {
			pt0.opacity = 0.1
		}
	}
 }

 Image {
	 id: pt1
	 x: 314
	 y: 291
	 width: 32
	 height: 32
	 opacity: 0.1
	 onXChanged: glview.vert1 = Qt.point(this.x + this.width * 0.5, this.y + this.height * 0.5)
	 onYChanged: glview.vert1 = Qt.point(this.x + this.width * 0.5, this.y + this.height * 0.5)
	 MouseArea {
		 id: mouse1
		 drag.maximumY: glview.height - pt1.height
		 drag.maximumX: glview.width - pt1.width
		 drag.minimumY: 0
		 drag.minimumX: 0
		 drag.axis: Drag.XAndYAxis
		 drag.target: pt1
		 anchors.fill: parent
		 onPressed: {
			 pt1.opacity = 0.8
		 }
		 onReleased: {
			 pt1.opacity = 0.1
		 }
	 }
	 source: "res/button.png"
 }

 Image {
	 id: pt2
	 x: 465
	 y: 291
	 width: 32
	 height: 32
	 opacity: 0.1
	 onXChanged: glview.vert2 = Qt.point(this.x + this.width * 0.5, this.y + this.height * 0.5)
	 onYChanged: glview.vert2 = Qt.point(this.x + this.width * 0.5, this.y + this.height * 0.5)
	 MouseArea {
		 id: mouse2
		 drag.maximumY: glview.height - pt2.height
		 drag.maximumX: glview.width - pt2.width
		 drag.minimumY: 0
		 drag.minimumX: 0
		 drag.axis: Drag.XAndYAxis
		 drag.target: pt2
		 anchors.fill: parent
		 onPressed: {
			 pt2.opacity = 0.8
		 }
		 onReleased: {
			 pt2.opacity = 0.1
		 }
	 }
	 source: "res/button.png"
 }
}
