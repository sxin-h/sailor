﻿using SailorEditor.ViewModels;
using SailorEditor.Services;
using SailorEditor.Controls;

namespace SailorEditor.Helpers
{
    public static class FolderTreeViewBuilder
    {
        private static TreeViewItemGroup<AssetFolder, AssetFile> FindParentFolder(TreeViewItemGroup<AssetFolder, AssetFile> group, AssetFolder folder)
        {
            if (group.GroupId == folder.ParentFolderId)
                return group;

            if (group.ChildrenGroups != null)
            {
                foreach (var currentGroup in group.ChildrenGroups)
                {
                    var search = FindParentFolder(currentGroup, folder);

                    if (search != null)
                        return search;
                }
            }

            return null;
        }
        public static TreeViewNode FindFileRecursive(this TreeViewNode parent, AssetFile id)
        {
            var assetFile = parent.BindingContext as TreeViewItem<AssetFile>;

            if (assetFile != null)
            {
                if (assetFile.Model.UID == id.UID)
                {
                    return parent;
                }
            }

            var assetFolder = parent.BindingContext as TreeViewItemGroup<AssetFolder, AssetFile>;
            if (assetFolder != null)
            {
                foreach (var el in parent.ChildrenList)
                {
                    var res = FindFileRecursive(el, id);

                    if (res != null)
                        return res;
                }
            }

            return null;
        }

        public static TreeViewItemGroup<AssetFolder, AssetFile> PopulateDirectory(AssetsService service)
        {
            var projectRoot = service.Root;
            var folders = service.Folders.OrderBy(x => x.ParentFolderId);
            var assets = service.Files;

            var projectRootGroup = new TreeViewItemGroup<AssetFolder, AssetFile>();
            projectRootGroup.Key = projectRoot.Name;

            foreach (var folder in folders)
            {
                var itemGroup = new TreeViewItemGroup<AssetFolder, AssetFile>();
                itemGroup.Model = folder;
                itemGroup.GroupId = folder.Id;
                itemGroup.Key = folder.Name;

                // Assets first
                var assetsInFolder = assets.Where(x => x.FolderId == folder.Id);

                foreach (var file in assetsInFolder)
                {
                    var item = new TreeViewItem<AssetFile>();
                    item.Model = file;
                    item.ItemId = file.Id;
                    item.Key = file.DisplayName;

                    itemGroup.ChildrenItems.Add(item);
                }

                // Folders now
                if (folder.ParentFolderId == -1)
                {
                    projectRootGroup.ChildrenGroups.Add(itemGroup);
                }
                else
                {
                    TreeViewItemGroup<AssetFolder, AssetFile> parentGroup = null;

                    foreach (var group in projectRootGroup.ChildrenGroups)
                    {
                        parentGroup = FindParentFolder(group, folder);

                        if (parentGroup != null)
                        {
                            parentGroup.ChildrenGroups.Add(itemGroup);
                            break;
                        }
                    }
                }
            }

            return projectRootGroup;
        }
    }
}