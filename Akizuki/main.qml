import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import io.qt.examples.backend 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true

	Material.theme: Material.Light
	Material.primary: Material.DeepOrange
    Material.accent: Material.Purple

    BackEnd {
        id: backend
    }

	Column {
		anchors.centerIn: parent

		signal buttonPressed

		Button { text: qsTr("Button +1") }
		Button { text: qsTr("Button +2") }
		Button { text: qsTr("Button -1") }
		Button { text: qsTr("Button -2") }

		TextField {
			text: backend.userName
			placeholderText: qsTr("User name")
			anchors.centerIn: parent

			onTextChanged: backend.userName = text
		}

		Text { text: backend.userName }

	}
}