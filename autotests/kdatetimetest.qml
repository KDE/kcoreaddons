/*
 * SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtTest
import org.kde.coreaddons as CoreAddons

Item {
    id: root

    property CoreAddons.KDateTime emptyDateTime
    property CoreAddons.KDateTime dt

    TestCase {
        name: "DateTimeTest"

        function init(): void {
            dt.dateTime = new Date(2024, 2, 15, 10, 30, 45);
        }

        function test_defaultIsInvalid(): void {
            compare(root.emptyDateTime.isValid, false);
        }

        function test_componentAccessors(): void {
            compare(dt.isValid, true);
            compare(dt.year, 2024);
            compare(dt.month, 3);
            compare(dt.day, 15);
            compare(dt.hour, 10);
            compare(dt.minute, 30);
            compare(dt.second, 45);
        }

        function test_writingAFieldSticks(): void {
            dt.year = 2025;
            compare(dt.year, 2025);
            compare(dt.month, 3);
            compare(dt.day, 15);

            dt.hour = 23;
            compare(dt.hour, 23);
            compare(dt.minute, 30);
        }

        function test_timeZoneIsReadable(): void {
            verify(dt.timeZone !== undefined);
        }

        function test_addHelpersReturnANewValue(): void {
            dt.dateTime = new Date(2024, 0, 31, 0, 0, 0);

            const plusOneDay = dt.addDays(1);
            compare(plusOneDay.day, 1);
            compare(plusOneDay.month, 2);

            compare(dt.day, 31);
            compare(dt.month, 1);
        }

        function test_isToday(): void {
            compare(CoreAddons.KDateTimeFactory.now().isToday, true);
            compare(dt.isToday, false);
        }

        function test_isCurrentMonthAndYear(): void {
            compare(CoreAddons.KDateTimeFactory.now().isCurrentMonth, true);
            compare(CoreAddons.KDateTimeFactory.now().isCurrentYear, true);
            compare(dt.isCurrentMonth, false);
            compare(dt.isCurrentYear, false);
        }

        function test_factoryNow(): void {
            compare(CoreAddons.KDateTimeFactory.now().isValid, true);
        }

        function test_factoryFromDateTime(): void {
            const fromFactory = CoreAddons.KDateTimeFactory.fromDateTime(new Date(2024, 2, 15, 10, 30, 45));
            compare(fromFactory.year, 2024);
            compare(fromFactory.month, 3);
            compare(fromFactory.day, 15);
        }

        function test_toLocaleDateString(): void {
            compare(dt.toLocaleDateString("yyyy"), "2024");
        }

        function test_startOfDay(): void {
            const startOfDay = dt.startOfDay();
            compare(startOfDay.day, dt.day);
            compare(startOfDay.hour, 0);
            compare(startOfDay.minute, 0);
        }

        function test_startOfEndOfMonthAndYear(): void {
            compare(dt.startOfMonth().day, 1);
            compare(dt.startOfMonth().month, dt.month);

            compare(dt.endOfMonth().day, 31); // dt is in March
            compare(dt.endOfYear().month, 12);
            compare(dt.endOfYear().day, 31);
        }

        function test_factoryInvalid(): void {
            compare(CoreAddons.KDateTimeFactory.invalid().isValid, false);
        }
    }
}
