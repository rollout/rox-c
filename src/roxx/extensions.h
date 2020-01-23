#pragma once

#include "parser.h"
#include "core/repositories.h"
#include "core/properties.h"

void ROX_INTERNAL add_experiments_extensions(
        Parser *parser,
        TargetGroupRepository *target_groups_repository,
        FlagRepository *flags_repository,
        ExperimentRepository *experiment_repository);

void ROX_INTERNAL add_properties_extensions(
        Parser *parser,
        CustomPropertyRepository *custom_property_repository,
        DynamicProperties *dynamic_properties);
