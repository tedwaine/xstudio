// SPDX-License-Identifier: Apache-2.0

#include "xstudio/session/enums.hpp"
#include "xstudio/ui/qml/QTreeModelToTableModel.hpp"
#include "xstudio/ui/qml/session_model_ui.hpp"

#include <QSignalSpy>
#include <QStandardItemModel>
#include <gtest/gtest.h>

using xstudio::ui::qml::MediaListFilterModel;
namespace session = xstudio::session;

namespace {
QStandardItem *mediaItem(const QString &name, bool selected) {
    auto *item = new QStandardItem(name);
    item->setData("Media", session::typeRole);
    item->setData(QVariantList{name}, session::mediaDisplayInfoRole);
    item->setData(selected, session::selectionRole);
    return item;
}
} // namespace

TEST(MediaListFilterModelTest, TracksSelectionAndVisibleRows) {
    QStandardItemModel source;
    source.appendRow(mediaItem("Alpha", true));
    source.appendRow(mediaItem("Beta", false));
    MediaListFilterModel proxy;
    proxy.setSourceModel(&source);
    QSignalSpy changed(&proxy, &MediaListFilterModel::selectedSourceIndexesChanged);

    EXPECT_EQ(proxy.selectedSourceIndexes(), QModelIndexList{source.index(0, 0)});
    EXPECT_EQ(
        proxy.property("selectedSourceIndexes").value<QModelIndexList>(),
        QModelIndexList{source.index(0, 0)});
    source.setData(source.index(1, 0), true, session::selectionRole);
    EXPECT_GT(changed.count(), 0);
    EXPECT_EQ(
        proxy.selectedSourceIndexes(),
        (QModelIndexList{source.index(0, 0), source.index(1, 0)}));

    changed.clear();
    proxy.setSearchString("Beta");
    EXPECT_GT(changed.count(), 0);
    EXPECT_EQ(proxy.selectedSourceIndexes(), QModelIndexList{source.index(1, 0)});
    source.setData(source.index(1, 0), false, session::selectionRole);
    EXPECT_TRUE(proxy.selectedSourceIndexes().isEmpty());

    proxy.setSearchString("");
    EXPECT_EQ(proxy.selectedSourceIndexes(), QModelIndexList{source.index(0, 0)});
}

TEST(MediaListFilterModelTest, TracksStructureAndSourceChanges) {
    QStandardItemModel source;
    auto *parent = mediaItem("Beta", true);
    parent->appendRow(mediaItem("Child", true));
    source.appendRow(parent);
    source.appendRow(mediaItem("Alpha", true));
    MediaListFilterModel proxy;
    proxy.setSourceModel(&source);
    QSignalSpy changed(&proxy, &MediaListFilterModel::selectedSourceIndexesChanged);

    EXPECT_EQ(
        proxy.selectedSourceIndexes(),
        (QModelIndexList{source.index(0, 0), source.index(1, 0)}));
    proxy.sort(0);
    EXPECT_GT(changed.count(), 0);
    EXPECT_EQ(proxy.selectedSourceIndexes().first(), source.index(1, 0));

    changed.clear();
    source.insertRow(0, mediaItem("Gamma", true));
    EXPECT_GT(changed.count(), 0);
    EXPECT_EQ(proxy.selectedSourceIndexes().size(), 3);
    source.removeRow(1);
    EXPECT_EQ(proxy.selectedSourceIndexes().size(), 2);

    QStandardItemModel replacement;
    replacement.appendRow(mediaItem("Replacement", true));
    changed.clear();
    proxy.setSourceModel(&replacement);
    EXPECT_GT(changed.count(), 0);
    EXPECT_EQ(proxy.selectedSourceIndexes(), QModelIndexList{replacement.index(0, 0)});
    changed.clear();
    replacement.clear();
    EXPECT_GT(changed.count(), 0);
    EXPECT_TRUE(proxy.selectedSourceIndexes().isEmpty());
    proxy.setSourceModel(nullptr);
}

TEST(MediaListFilterModelTest, ReturnsIndexesInImmediateSourceModel) {
    QStandardItemModel source;
    source.appendRow(mediaItem("Alpha", true));
    QTreeModelToTableModel table;
    table.setModel(&source);
    MediaListFilterModel proxy;
    proxy.setSourceModel(&table);

    EXPECT_EQ(proxy.selectedSourceIndexes(), QModelIndexList{table.index(0, 0)});
}
