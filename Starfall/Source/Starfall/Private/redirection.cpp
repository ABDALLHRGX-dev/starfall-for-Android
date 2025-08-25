#include "redirection.h"
#include "opts.h"

namespace Starfall {
    namespace Redirection {
        bool shouldRedirect(URL* uri) {
            switch (URLSet) {
                case StarfallURLSet::Hybrid: {
                    constexpr static const wchar_t* redirectedPaths[] = {
                        L"/fortnite/api/v2/versioncheck/",
                        L"/fortnite/api/game/v2/profile/",
                        L"/content/api/pages/",
                        L"/affiliate/api/public/affiliates/slug",
                        L"/socialban/api/public/v1",
                        L"/fortnite/api/cloudstorage/system"
                    };

                    for (int i = 0; i < sizeof(redirectedPaths) / sizeof(wchar_t*); i++) {
                        if (uri->Path.starts_with(redirectedPaths[i])) return true;
                    }
                    break;
                }
                case StarfallURLSet::Dev: {
                    constexpr static const wchar_t* redirectedPathsDev[] = {
                        L"/fortnite/api/game/v2/profile/",
                        L"/affiliate/api/public/affiliates/slug",
                        L"/content/api/pages/"
                    };

                    for (int i = 0; i < sizeof(redirectedPathsDev) / sizeof(wchar_t*); i++) {
                        if (uri->Path.starts_with(redirectedPathsDev[i])) return true;
                    }
                    break;
                }
                case StarfallURLSet::Default: {
                    static constexpr const wchar_t *redirectedUrls[] = {
                        L"ol.epicgames.com",
                        L"ol.epicgames.net",
                        L"on.epicgames.com",
                        L"game-social.epicgames.com",
                        L"ak.epicgames.com",
                        L"epicgames.dev"
                    };

                    for (int i = 0; i < sizeof(redirectedUrls) / sizeof(wchar_t *); i++) {
                        if (uri->Domain.ends_with(redirectedUrls[i])) return true;
                    }
                    break;
                }
                case StarfallURLSet::All:
                    return true;
           }
            return false;
        }
    }
}
