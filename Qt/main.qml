import QtQuick 2.2
import QtQuick.Window 2.1
import OpenGLUnderQML 1.0
import QtQuick.Controls 1.2

Item {
	id: item1
	width: 800
	height: 600
	visible: true

	OpenGLView {
		id: glview
		anchors.fill: parent
		visible: true
	}

 Image {
	id: pt0
	x: 357
	y: 138
	width: 32
	height: 32
	source: "res/button.png"

	 MouseArea {
		id: mouse0
		anchors.fill: parent
		drag.target: pt0
		drag.axis: Drag.XAndYAxis
		drag.minimumX: 0
		drag.minimumY: 0
		drag.maximumX: glview.width - pt0.width
		drag.maximumY: glview.height - pt0.height
	 }
 }

 Image {
	 id: pt1
	 x: 279
	 y: 291
	 width: 32
	 height: 32
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
