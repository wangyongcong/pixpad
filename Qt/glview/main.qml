import QtQuick 2.2
import QtQuick.Window 2.1
import OpenGLUnderQML 1.0

Item {
	visible: true
	width: 360
	height: 360

	MouseArea {
		anchors.fill: parent
		onClicked: {
			Qt.quit();
		}
	}

	OpenGLView {
		visible: true
		x: 59
		y: 40
		width: 242
		height: 281

	}

	Text {
		text: qsTr("Hello World")
		anchors.centerIn: parent
	}
}
