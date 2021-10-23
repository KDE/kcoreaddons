#
# SPDX-FileCopyrightText: 2016 Shaheed Haque <srhaque@theiet.org>
# SPDX-FileCopyrightText: 2016 Stephen Kelly <steveire@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

import os, sys

import rules_engine
sys.path.append(os.path.dirname(os.path.dirname(rules_engine.__file__)))
import Qt5Ruleset

from clang.cindex import AccessSpecifier

def discard_base(container, sip, matcher):
    sip["base_specifiers"] = []

def local_container_rules():
    return [
        [".*", "KUserId", ".*", ".*", ".*", discard_base],
        [".*", "KGroupId", ".*", ".*", ".*", discard_base]
    ]

def local_forward_declaration_rules():
    return [
        # [".*", "QWidget", ".*", rules_engine.mark_forward_declaration_external]
    ]

def local_function_rules():
    return [

        ["KUser", "KUser", ".*", ".*", ".*passwd.*", rules_engine.function_discard],
        ["KUserGroup", "KUserGroup", ".*", ".*", ".*group.*", rules_engine.function_discard],

        ["KCrash", "defaultCrashHandler", ".*", ".*", ".*", rules_engine.function_discard],

        ["KPluginFactory", "create", ".*", ".*", ".*QWidget.*", rules_engine.function_discard],
        ["KPluginFactory", "loadFactory", ".*", ".*", ".*", rules_engine.function_discard],
        ["KPluginMetaData", ".*", ".*", ".*", "QStaticPlugin.*", rules_engine.function_discard],

        [".*", ".*", ".*", ".*", ".*QStringView.*", rules_engine.function_discard],

        # Deprecated
        ["KPluginFactory", "createPartObject", ".*", ".*", ".*", rules_engine.function_discard],

        # Multiple overloads with same python signature
        ["KMacroExpanderBase", "expandMacrosShellQuote", ".*", ".*", ".*int.*", rules_engine.function_discard],
        ["KRandomSequence", "setSeed", ".*", ".*", "int.*", rules_engine.function_discard],
        ["KPluginMetaData", "value", ".*", ".*", ".*int.*", rules_engine.function_discard],
        ["KPluginMetaData", "value", ".*", ".*", ".*bool.*", rules_engine.function_discard],
        ["KPluginMetaData", "value", ".*", ".*", ".*char.*", rules_engine.function_discard],
        ["KPluginMetaData", "value", ".*", ".*", ".*QStringList.*", rules_engine.function_discard],

        [".*", "qobject_cast", ".*", ".*", ".*", rules_engine.function_discard],
        [".*", "qobject_interface_iid", ".*", ".*", ".*", rules_engine.function_discard],
    ]

def rewrite_typedef_as_unsigned_int(container, typedef, sip, matcher):
    sip["decl"] = "unsigned int"

def local_typedef_rules():
    return [
        ["KPluginFactory", "CreateInstanceFunction", rules_engine.typedef_discard],
        [".*", "K_GID", rewrite_typedef_as_unsigned_int],
        [".*", "K_UID", rewrite_typedef_as_unsigned_int],
    ]

class RuleSet(Qt5Ruleset.RuleSet):
    def __init__(self):
        Qt5Ruleset.RuleSet.__init__(self)
        self._forward_declaration_db = rules_engine.ForwardDeclarationRuleDb(lambda: local_forward_declaration_rules() + Qt5Ruleset.forward_declaration_rules())
        self._fn_db = rules_engine.FunctionRuleDb(lambda: local_function_rules() + Qt5Ruleset.function_rules())
        self._container_db = rules_engine.ContainerRuleDb(lambda: local_container_rules() + Qt5Ruleset.container_rules())
        self._typedef_db = rules_engine.TypedefRuleDb(lambda: local_typedef_rules() + Qt5Ruleset.typedef_rules())
