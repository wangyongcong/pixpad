import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import QMLOpenGL 1.0

MeshView {
	id: glview
	width: 800
	height: 600
	anchors.fill: parent
	signal vertsChanged(int idx, int x, int y)

 Image {
	id: pt0
	x: 384
	y: 138
	width: 32
	height: 32
	source: "res/button.png"
	opacity: 0.1

	onXChanged: glview.vertsChanged(0, this.x, this.y)
	onYChanged: glview.vertsChanged(0, this.x, this.y)

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
			pt0.opacity = 0.5
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
	 onXChanged: glview.vertsChanged(1, this.x, this.y)
	 onYChanged: glview.vertsChanged(1, this.x, this.y)
	 MouseArea {
		 id: mouse1
		 drag.maximumY: glview.height - pt1.height
		 drag.maximumX: glview.width - pt1.width
		 drag.minimumY: 0
		 drag.minimumX: 0
		 drag.axis: Drag.XAndYAxis
		 drag.target: pt1
		 anchors.fill: parent
	 }
	 source: "res/button.png"
 }

 Image {
	 id: pt2
	 x: 465
	 y: 291
	 width: 32
	 height: 32
	 onXChanged: glview.vertsChanged(2, this.x, this.y)
	 onYChanged: glview.vertsChanged(2, this.x, this.y)
	 MouseArea {
		 id: mouse2
		 drag.maximumY: glview.height - pt2.height
		 drag.maximumX: glview.width - pt2.width
		 drag.minimumY: 0
		 drag.minimumX: 0
		 drag.axis: Drag.XAndYAxis
		 drag.target: pt2
		 anchors.fill: parent
	 }
	 source: "res/button.png"
 }
}
