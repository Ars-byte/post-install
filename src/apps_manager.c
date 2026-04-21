#include "apps_manager.h"
#include <stddef.h>

static AppInfo apps[] = {
    {"Steam", "steam.png", "pkexec xbps-install -Syu void-repo-nonfree void-repo-multilib void-repo-multilib-nonfree && pkexec xbps-install -Syu steam-udev-rules libGL-32bit libpulseaudio-32bit libtxc_dxtn-32bit mesa-dri mesa-dri-32bit && flatpak install flathub com.valvesoftware.Steam -y", GROUP_GAMING, FALSE, FALSE},
    {"ProtonUp-Qt", "protonup-qt.png", "flatpak install flathub net.davidotek.pupgui2 -y", GROUP_GAMING, FALSE, FALSE},
    {"PortProton", "portproton.png", "pkexec xbps-install -Sy portproton", GROUP_GAMING, FALSE, FALSE},
    {"Heroic Games Launcher", "heroic.png", "pkexec xbps-install -Sy heroic-games", GROUP_GAMING, FALSE, FALSE},
    {"Faugus Launcher", "faugus.png", "pkexec xbps-install -Sy faugus-launcher", GROUP_GAMING, FALSE, FALSE},
    {"Hytale", "hytale.png", "wget -O /tmp/tmp.flatpak https://launcher.hytale.com/builds/release/linux/amd64/hytale-launcher-latest.flatpak && flatpak install /tmp/tmp.flatpak -y", GROUP_GAMING, FALSE, FALSE},
    {"ElyPrismLauncher", "elyprismlauncher.png", "pkexec xbps-install -Sy elyprismlauncher", GROUP_GAMING, FALSE, FALSE},
    {"Trinity Launcher", "trinity.png", "flatpak install com.trench.trinity.launcher -y", GROUP_GAMING, FALSE, FALSE},
    
    {"Reaper", "reaper.png", "flatpak install flathub fm.reaper.Reaper -y", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"OBS Studio", "obs.png", "flatpak install flathub com.obsproject.Studio -y", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"Kdenlive", "kdenlive.png", "flatpak install flathub org.kde.kdenlive -y", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"OpenShot", "org.openshot.OpenShot.png", "flatpak install flathub org.openshot.OpenShot -y", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"VLC", "vlc.png", "pkexec xbps-install -Sy vlc", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"Audacity", "audacity-logo.png", "pkexec xbps-install -Sy audacity", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"Ardour", "ardour.png", "pkexec xbps-install -Sy ardour", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    {"Blender", "Blender.png", "pkexec xbps-install -Sy blender", GROUP_AUDIO_VIDEO, FALSE, FALSE},
    
    {"Krita", "krita.png", "flatpak install flathub org.kde.krita -y", GROUP_DRAWING_IMAGE, FALSE, FALSE},
    {"GIMP", "gimp.png", "pkexec xbps-install -Sy gimp", GROUP_DRAWING_IMAGE, FALSE, FALSE},
    {"Inkscape", "Inkscape.png", "pkexec xbps-install -Sy inkscape", GROUP_DRAWING_IMAGE, FALSE, FALSE},
    
    {"Spotify", "Spotify_icon.svg.png", "flatpak install flathub com.spotify.Client -y", GROUP_SOCIAL, FALSE, FALSE},
    {"Vesktop", "vesktop.png", "pkexec xbps-install -Sy vesktop", GROUP_SOCIAL, FALSE, FALSE},
    {"Waterfox", "waterfox.png", "pkexec xbps-install -Sy waterfox", GROUP_SOCIAL, FALSE, FALSE},
    {"Brave", "brave.png", "pkexec xbps-install -Sy brave-bin", GROUP_SOCIAL, FALSE, FALSE},
    {"Zerotierone", "zerotierone.png", "pkexec xbps-install -Sy zerotierone", GROUP_SOCIAL, FALSE, FALSE},
    {"Telegram", "telegram.png", "flatpak install flathub org.telegram.desktop -y", GROUP_SOCIAL, FALSE, FALSE},
    {"Vivaldi", "vivaldi.png", "pkexec xbps-install -Sy vivaldi", GROUP_SOCIAL, FALSE, FALSE},
    {"Chromium", "chromium.png", "pkexec xbps-install -Sy chromium", GROUP_SOCIAL, FALSE, FALSE},
    
    {"OnlyOffice", "onlyoffice.png", "flatpak install flathub org.onlyoffice.desktopeditors -y", GROUP_TEXT_DOCUMENTS, FALSE, FALSE},
    {"Kate", "org.kde.kate.desktop.png", "pkexec xbps-install -Sy kate", GROUP_TEXT_DOCUMENTS, FALSE, FALSE},
    {"LibreOffice", "Libre-Office.png", "pkexec xbps-install -Sy libreoffice", GROUP_TEXT_DOCUMENTS, FALSE, FALSE},
    {"AMD Drivers", "amd.png", "pkexec xbps-install -Sy mesa-dri mesa-dri-32bit mesa-vulkan-radeon mesa-vulkan-radeon-32bit linux-firmware-amd", GROUP_DRIVERS, FALSE, FALSE},
    {"Intel Drivers", "intel.png", "pkexec xbps-install -Sy mesa-dri mesa-dri-32bit mesa-vulkan-intel mesa-vulkan-intel-32bit linux-firmware-intel libva-intel-driver intel-media-driver mesa-intel-dri-32bit mesa-intel-dri", GROUP_DRIVERS, FALSE, FALSE},
    {"Nvidia Nouveau", "nvidia.png", "pkexec xbps-install -Sy mesa-dri mesa-dri-32bit xf86-video-nouveau mesa-vulkan-nouveau", GROUP_DRIVERS, FALSE, FALSE},
    {"Nvidia Proprietary", "nvidia.png", "pkexec xbps-install -Sy mesa-dri mesa-dri-32bit nvidia nvidia-dkms nvidia-firmware nvidia-gtklibs nvidia-gtklibs-32bit nvidia-libs nvidia-libs-32bit nvidia-opencl nvidia-opencl-32bit nvidia-vaapi-driver nvidia-docker nvidia-container-toolkit nvidia390 nvidia390-dkms nvidia390-gtklibs nvidia390-gtklibs-32bit nvidia390-libs nvidia390-libs-32bit nvidia390-opencl nvidia390-opencl-32bit nvidia470 nvidia470-dkms nvidia470-gtklibs nvidia470-libs nvidia470-libs-32bit nvidia470-opencl mesa-nouveau-dri mesa-nouveau-dri-32bit", GROUP_DRIVERS, FALSE, FALSE}
};

GList *get_all_apps(void) {
    GList *list = NULL;
    int num_apps = G_N_ELEMENTS(apps);
    for (int i = 0; i < num_apps; i++) {
        list = g_list_prepend(list, &apps[i]);
    }
    return g_list_reverse(list);
}

static gchar *cached_resource_dir = NULL;

const char *get_base_resource_dir(void) {
    if (cached_resource_dir) return cached_resource_dir;
    
    gchar *exe_path = g_file_read_link("/proc/self/exe", NULL);
    gchar *exe_dir = exe_path ? g_path_get_dirname(exe_path) : g_get_current_dir();
    g_free(exe_path);
    
    gchar *parent_dir = g_build_filename(exe_dir, "..", NULL);
    
    const char *bases[] = {
        exe_dir,
        parent_dir,
        "/usr/share/neko-store",
        "/opt/neko-store",
        "/home/javierc/Documentos/server/dev/Neko Store"
    };
    
    for (int i = 0; i < G_N_ELEMENTS(bases); i++) {
        gchar *test_file = g_build_filename(bases[i], "data", "style.css", NULL);
        if (g_file_test(test_file, G_FILE_TEST_EXISTS)) {
            cached_resource_dir = g_strdup(bases[i]);
            g_free(test_file);
            break;
        }
        g_free(test_file);
    }
    
    if (!cached_resource_dir) {
        cached_resource_dir = g_strdup(exe_dir);
    }
    
    g_free(parent_dir);
    g_free(exe_dir);
    
    return cached_resource_dir;
}

gchar *get_resource_path(const char *rel_path) {
    return g_build_filename(get_base_resource_dir(), rel_path, NULL);
}
