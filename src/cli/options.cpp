/**
 * @file options.cpp
 * @brief CLI 选项实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/options.hpp"

namespace czc::cli {

namespace {

/// 全局选项实例
CliOptions g_options;

} // namespace

CliOptions &cliOptions() noexcept { return g_options; }

const CliOptions &cliOptionsConst() noexcept { return g_options; }

void resetOptions() noexcept { g_options = CliOptions{}; }

} // namespace czc::cli
