// SPDX-License-Identifier: Apache-2.0
import xstudio.qml.helpers 1.0

XsPreferenceMap {
	id: control
	property string path: ""

	onPathChanged: index = globalStoreModel.searchRecursive(path, "pathRole")
}

