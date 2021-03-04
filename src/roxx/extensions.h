#pragma once

#include "rox/macros.h"
#include "parser.h"
#include "core/repositories.h"
#include "core/properties.h"

ROX_INTERNAL double experiment_extensions_get_bucket(const char *seed);

/**
 * @param parser Not <code>NULL</code>.
 * @param target_groups_repository Not <code>NULL</code>.
 * @param flags_repository Not <code>NULL</code>.
 * @param experiment_repository Not <code>NULL</code>.
 */
ROX_INTERNAL void parser_add_experiments_extensions(
        Parser *parser,
        TargetGroupRepository *target_groups_repository,
        FlagRepository *flags_repository,
        ExperimentRepository *experiment_repository);

/**
 * @param parser Not <code>NULL</code>.
 * @param custom_property_repository Not <code>NULL</code>.
 * @param dynamic_properties Not <code>NULL</code>.
 */
ROX_INTERNAL void parser_add_properties_extensions(
        Parser *parser,
        CustomPropertyRepository *custom_property_repository,
        DynamicProperties *dynamic_properties);
