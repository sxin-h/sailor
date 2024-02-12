﻿using Microsoft.Maui.Controls.Compatibility;
using SailorEditor.Engine;
using SailorEditor.Helpers;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using YamlDotNet.RepresentationModel;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.Serialization;
using SailorEditor.Services;

namespace SailorEditor.ViewModels
{
    using AssetUID = string;

    public class AssetFile : INotifyPropertyChanged
    {
        public AssetUID UID { get { return Properties["fileId"].ToString(); } }
        public FileInfo Asset { get; set; }
        public FileInfo AssetInfo { get; set; }
        public Dictionary<string, object> Properties { get; set; } = new();

        public string DisplayName
        {
            get { return displayName; }
            set
            {
                if (displayName != value)
                {
                    displayName = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(DisplayName));
                }
            }
        }

        public int Id { get; set; }
        public int FolderId { get; set; }
        public bool IsDirty
        {
            get { return isDirty; }
            set
            {
                if (isDirty != value)
                {
                    isDirty = value;

                    OnPropertyChanged(nameof(IsDirty));
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        public virtual bool PreloadResources(bool force) => true;
        public void UpdateAssetFile()
        {
            UpdateModel();

            using (var yamlAssetInfo = new FileStream(AssetInfo.FullName, FileMode.Create))
            using (var writer = new StreamWriter(yamlAssetInfo))
            {
                var serializer = new SerializerBuilder()
                    .WithNamingConvention(CamelCaseNamingConvention.Instance)
                    .Build();

                var yaml = serializer.Serialize(Properties);
                writer.Write(yaml);
            }

            IsDirty = false;
        }
        public void Revert()
        {
            PreloadResources(true);
            IsDirty = false;
        }

        protected bool IsLoaded { get; set; }
        protected virtual void UpdateModel() { }

        bool isDirty = false;
        string displayName;
    }
    public class ModelFile : AssetFile
    {
        public bool ShouldGenerateMaterials
        {
            get { return shouldGenerateMaterials; }
            set
            {
                if (shouldGenerateMaterials != value)
                {
                    shouldGenerateMaterials = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(ShouldGenerateMaterials));
                }
            }
        }
        public bool ShouldBatchByMaterial
        {
            get { return shouldBatchByMaterial; }
            set
            {
                if (shouldBatchByMaterial != value)
                {
                    shouldBatchByMaterial = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(ShouldBatchByMaterial));
                }
            }
        }

        public List<AssetUID> DefaultMaterials
        {
            get { return defaultMaterials; }
            set
            {
                if (DefaultMaterials != value)
                {
                    defaultMaterials = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(DefaultMaterials));
                }
            }
        }

        private bool shouldGenerateMaterials;
        private bool shouldBatchByMaterial;
        private List<AssetUID> defaultMaterials = new();

        protected override void UpdateModel()
        {
            Properties["bShouldGenerateMaterials"] = ShouldGenerateMaterials.ToString();
            Properties["bShouldBatchByMaterial"] = ShouldBatchByMaterial.ToString();
            Properties["defaultMaterials"] = defaultMaterials.ToString();

            IsDirty = false;
        }
        public override bool PreloadResources(bool force)
        {
            if (!IsLoaded || force)
            {
                try
                {
                    foreach (var e in Properties)
                    {
                        switch (e.Key.ToString())
                        {
                            case "bShouldGenerateMaterials":
                                ShouldGenerateMaterials = bool.Parse(e.Value.ToString());
                                break;
                            case "bShouldBatchByMaterial":
                                ShouldBatchByMaterial = bool.Parse(e.Value.ToString());
                                break;
                            case "defaultMaterials":
                                {
                                    var parsed = new List<AssetUID>();
                                    bool ignoreFirst = false;
                                    foreach (var uid in (e.Value as YamlNode).AllNodes)
                                    {
                                        if (!ignoreFirst)
                                        {
                                            ignoreFirst = true;
                                            continue;
                                        }

                                        parsed.Add((AssetUID)uid.ToString());
                                    }

                                    DefaultMaterials = parsed;
                                }
                                break;
                        }
                    }
                }
                catch (Exception e)
                {
                    DisplayName = e.Message;
                    return false;
                }

                IsDirty = false;
                IsLoaded = true;
            }

            return true;
        }
    }
    public class TextureFile : AssetFile
    {
        public ImageSource Texture { get; protected set; }
        public bool ShouldGenerateMips
        {
            get { return shouldGenerateMips; }
            set
            {
                if (shouldGenerateMips != value)
                {
                    shouldGenerateMips = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(ShouldGenerateMips));
                }
            }
        }
        public bool ShouldSupportStorageBinding
        {
            get { return shouldSupportStorageBinding; }
            set
            {
                if (shouldSupportStorageBinding != value)
                {
                    shouldSupportStorageBinding = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(ShouldSupportStorageBinding));
                }
            }
        }
        public TextureClamping Clamping
        {
            get { return clamping; }
            set
            {
                if (clamping != value)
                {
                    clamping = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(Clamping));
                }
            }
        }
        public TextureFiltration Filtration
        {
            get { return filtration; }
            set
            {
                if (filtration != value)
                {
                    filtration = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(Filtration));
                }
            }
        }
        public TextureFormat Format
        {
            get { return format; }
            set
            {
                if (format != value)
                {
                    format = value;
                    IsDirty = true;

                    OnPropertyChanged(nameof(Format));
                }
            }
        }

        private TextureFiltration filtration;
        private TextureFormat format = TextureFormat.R8G8B8A8_SRGB;
        private TextureClamping clamping;
        private bool shouldGenerateMips;
        private bool shouldSupportStorageBinding;
        protected override void UpdateModel()
        {
            Properties["bShouldGenerateMips"] = ShouldGenerateMips.ToString();
            Properties["bShouldSupportStorageBinding"] = ShouldSupportStorageBinding.ToString();
            Properties["clamping"] = Clamping.ToString();
            Properties["filtration"] = Filtration.ToString();
            Properties["format"] = Format.ToString();

            IsDirty = false;
        }

        public override bool PreloadResources(bool force)
        {
            if (!IsLoaded || force)
            {
                try
                {
                    Texture = ImageSource.FromFile(Asset.FullName);
                }
                catch (Exception ex)
                {
                    DisplayName = ex.Message;
                }

                try
                {
                    foreach (var e in Properties)
                    {
                        switch (e.Key.ToString())
                        {
                            case "bShouldGenerateMips":
                                ShouldGenerateMips = bool.Parse(e.Value.ToString());
                                break;
                            case "bShouldSupportStorageBinding":
                                ShouldSupportStorageBinding = bool.Parse(e.Value.ToString());
                                break;
                            case "clamping":
                                {
                                    TextureClamping outEnum;
                                    if (Enum.TryParse(e.Value.ToString(), out outEnum))
                                        Clamping = outEnum;
                                    else
                                        return false;
                                    break;
                                }
                            case "filtration":
                                {
                                    TextureFiltration outEnum;
                                    if (Enum.TryParse(e.Value.ToString(), out outEnum))
                                        Filtration = outEnum;
                                    else
                                        return false;
                                    break;
                                }
                            case "format":
                                {
                                    TextureFormat outEnum;
                                    if (Enum.TryParse(e.Value.ToString(), out outEnum))
                                        Format = outEnum;
                                    else
                                        return false;
                                    break;
                                }
                        }
                    }
                }
                catch (Exception e)
                {
                    DisplayName = e.Message;
                    return false;
                }

                IsDirty = false;
                IsLoaded = true;
            }

            return true;
        }
    }
    public class ShaderLibraryFile : AssetFile
    {
        public string Code { get; set; }

        public override bool PreloadResources(bool force)
        {
            if (IsLoaded && !force)
                return true;

            Code = File.ReadAllText(Asset.FullName);

            IsLoaded = true;
            return true;
        }
    }

    public class ShaderFile : AssetFile
    {
        public string Includes { get; set; }
        public string Defines { get; set; }
        public string GlslCommonShader { get; set; }
        public string GlslVertexShader { get; set; }
        public string GlslFragmentShader { get; set; }
        public string GlslComputeShader { get; set; }

        public override bool PreloadResources(bool force)
        {
            if (IsLoaded && !force)
                return true;

            try
            {
                using (var yamlAssetInfo = new FileStream(Asset.FullName, FileMode.Open))
                using (var reader = new StreamReader(yamlAssetInfo))
                {
                    var yaml = new YamlStream();
                    yaml.Load(reader);

                    var root = (YamlMappingNode)yaml.Documents[0].RootNode;

                    foreach (var e in root.Children)
                    {
                        switch (e.Key.ToString())
                        {
                            case "includes":
                                Includes += e.Value.ToString() + "\n";
                                break;
                            case "defines":
                                Defines += e.Value.ToString() + "\n";
                                break;
                            case "glslVertex":
                                GlslVertexShader = e.Value.ToString();
                                break;
                            case "glslFragment":
                                GlslFragmentShader = e.Value.ToString();
                                break;
                            case "glslCommon":
                                GlslCommonShader = e.Value.ToString();
                                break;
                            case "glslCompute":
                                GlslComputeShader = e.Value.ToString();
                                break;
                        }
                    }

                    yamlAssetInfo.Close();
                }
            }
            catch (Exception e)
            {
                DisplayName = e.ToString();
                return false;
            }

            IsLoaded = true;

            return true;
        }
    }
}