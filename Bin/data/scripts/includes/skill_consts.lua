-- Skill type tree
SkillTypeSkill = 0
SkillTypeAttack = 1
    SkillTypeRangedAttack        = SkillTypeAttack | 1 << 8
        SkillTypeBowAttack           = SkillTypeRangedAttack | 1 << 16
        SkillTypeSpearAttack         = SkillTypeRangedAttack | 2 << 16
    SkillTypeMeleeAttack         = SkillTypeAttack | 2 << 8
        SkillTypeAxeAttack           = SkillTypeMeleeAttack | 1 << 16
        SkillTypeDaggerAttack        = SkillTypeMeleeAttack | 2 << 16
            SkillTypeLeadAttack          = SkillTypeDaggerAttack | 1 << 24
            SkillTypeOffHandAttack       = SkillTypeDaggerAttack | 2 << 24
            SkillTypeDualAttack          = SkillTypeDaggerAttack | 3 << 24
        SkillTypeHammerAttack        = SkillTypeMeleeAttack | 3 << 16
        SkillTypeScyteAttack         = SkillTypeMeleeAttack | 4 << 16
        SkillTypeSwordAttack         = SkillTypeMeleeAttack | 5 << 16
    SkillTypePetAttack           = SkillTypeAttack | 3 << 8
SkillTypeShout = 2
SkillTypeChant = 3
SkillTypeEcho = 4
SkillTypeForm = 5
SkillTypeGlypthe = 6
SkillTypePreparation = 7
SkillTypeRitual = 8
    SkillTypeBindingRitual       = SkillTypeRitual | 1 << 8
    SkillTypeNatureRitual        = SkillTypeRitual | 2 << 8
SkillTypeSignet = 9
SkillTypeSpell = 10
    SkillTypeEnchantment         = SkillTypeSpell | 1 << 8
        SkillTypeFlashEnchantment    = SkillTypeEnchantment | 1 << 16
    SkillTypeHex                 = SkillTypeSpell | 2 << 8
    SkillTypeItemSpell           = SkillTypeSpell | 3 << 8
    SkillTypeWardSpell           = SkillTypeSpell | 4 << 8
    SkillTypeWeaponSpell         = SkillTypeSpell | 5 << 8
    SkillTypeWellSpell           = SkillTypeSpell | 6 << 8
SkillTypeStance = 11
SkillTypeTrap = 12

-- Effects skills can have
SkillEffectNone         = 0
SkillEffectResurrect    = 1
SkillEffectHeal         = 2
SkillEffectProtect      = 3
SkillEffectDamage       = 4

SkillTargetNone         = 0
SkillTargetSelf         = 1 << 8
SkillTargetTarget       = 2 << 8
SkillTargetAoe          = 3 << 8