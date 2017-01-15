#
# Copyright 2016 by Shaheed Haque (srhaque@theiet.org)
# Copyright 2016 Stephen Kelly <steveire@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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

        # Deprecated
        ["KPluginFactory", "createPartObject", ".*", ".*", ".*", rules_engine.function_discard],

        # Multiple overloads with same python signature
        ["KMacroExpanderBase", "expandMacrosShellQuote", ".*", ".*", ".*, int ", rules_engine.function_discard],
        ["KRandomSequence", "setSeed", ".*", ".*", "int.*", rules_engine.function_discard],

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
