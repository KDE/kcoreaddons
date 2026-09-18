/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQml
import QtTest 1.2
import org.kde.coreaddons as KCoreAddons

TestCase {
    name: "KFormatQml"

    function test_formatTime() {
        if (_obj.systemTimeZone !== "Asia/Kolkata") {
            skip("timezone change not supported on this platform");
        }
        compare(KCoreAddons.Format.formatTime(_obj, "euTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "12:34 CEST");
        compare(KCoreAddons.Format.formatTime(_obj, "indiaTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "12:34");
        compare(KCoreAddons.Format.formatTime(_obj, "utcTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "12:34 UTC");

        compare(KCoreAddons.Format.formatTime(_obj, "indiaTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviation), "12:34 IST");
        compare(KCoreAddons.Format.formatTime(_obj, "utcTime", Locale.ShortFormat), "12:34");
        compare(KCoreAddons.Format.formatTime(_obj, "euTime"), "12:34");

        compare(KCoreAddons.Format.formatTime(_obj, "invalid", Locale.ShortFormat), "");
        compare(KCoreAddons.Format.formatTime(null, "invalid", Locale.ShortFormat), "");
    }

    function test_formatDateTime() {
        if (_obj.systemTimeZone !== "Asia/Kolkata") {
            skip("timezone change not supported on this platform");
        }

        // now testing formatDateTime() shortformat
        compare(KCoreAddons.Format.formatDateTime(_obj, "euTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "29/05/2025 12:34 CEST");
        compare(KCoreAddons.Format.formatDateTime(_obj, "indiaTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "29/05/2025 12:34");
        compare(KCoreAddons.Format.formatDateTime(_obj, "utcTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "29/05/2025 12:34 UTC");

        compare(KCoreAddons.Format.formatDateTime(_obj, "indiaTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviation), "29/05/2025 12:34 IST");
        compare(KCoreAddons.Format.formatDateTime(_obj, "utcTime", Locale.ShortFormat), "29/05/2025 12:34");
        compare(KCoreAddons.Format.formatDateTime(_obj, "euTime"), "29/05/2025 12:34");

        compare(KCoreAddons.Format.formatDateTime(_obj, "invalid", Locale.ShortFormat), "");
        compare(KCoreAddons.Format.formatDateTime(null, "invalid", Locale.ShortFormat), "");

        // now testing formatDateTime() longformat
        compare(KCoreAddons.Format.formatDateTime(_obj, "euTime", Locale.LongFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "Thursday, 29 May 2025 12:34:00 CEST");
        compare(KCoreAddons.Format.formatDateTime(_obj, "indiaTime", Locale.LongFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "Thursday, 29 May 2025 12:34:00");
        compare(KCoreAddons.Format.formatDateTime(_obj, "utcTime", Locale.LongFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "Thursday, 29 May 2025 12:34:00 UTC");

        compare(KCoreAddons.Format.formatDateTime(_obj, "indiaTime", Locale.LongFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviation), "Thursday, 29 May 2025 12:34:00 IST");
        compare(KCoreAddons.Format.formatDateTime(_obj, "utcTime", Locale.LongFormat), "Thursday, 29 May 2025 12:34:00");
        compare(KCoreAddons.Format.formatDateTime(_obj, "euTime", Locale.LongFormat), "Thursday, 29 May 2025 12:34:00");

        compare(KCoreAddons.Format.formatDateTime(_obj, "invalid", Locale.LongFormat), "");
        compare(KCoreAddons.Format.formatDateTime(null, "invalid", Locale.LongFormat), "");
    }
}
