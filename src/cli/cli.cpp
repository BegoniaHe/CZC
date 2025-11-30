/**
 * @file cli.cpp
 * @brief CLI 主入口实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/cli.hpp"
#include "czc/cli/commands/lex_command.hpp"
#include "czc/cli/commands/version_command.hpp"
#include "czc/cli/options.hpp"

#include <iostream>

namespace czc::cli {

Cli::Cli() : app_(std::string(kProgramDescription), std::string(kProgramName)) {
  // 设置版本标志
  app_.set_version_flag("--version,-V", std::string(kProgramName) +
                                            " version " +
                                            std::string(kVersion));

  // 要求至少一个子命令
  app_.require_subcommand(1);

  // 设置全局选项
  setupGlobalOptions();

  // 注册子命令
  registerCommands();
}

int Cli::run(int argc, char **argv) {
  try {
    app_.parse(argc, argv);

    // 执行激活的命令
    if (activeCommand_ != nullptr) {
      auto result = activeCommand_->execute();
      if (result.has_value()) {
        return result.value();
      }
      // 输出错误信息
      std::cerr << "Error: " << result.error().format() << "\n";
      return 1;
    }

    return 0;
  } catch (const CLI::ParseError &e) {
    return app_.exit(e);
  }
}

void Cli::registerCommands() {
  registerCommand<VersionCommand>();
  registerCommand<LexCommand>();
}

void Cli::setupGlobalOptions() {
  auto &opts = cliOptions();

  // 详细输出选项
  app_.add_flag(
          "-v,--verbose",
          [&opts](std::int64_t count) {
            if (count > 0) {
              opts.global.logLevel = LogLevel::Verbose;
            }
          },
          "Enable verbose output")
      ->group("Global Options");

  // 静默模式
  app_.add_flag(
          "-q,--quiet",
          [&opts](std::int64_t count) {
            if (count > 0) {
              opts.global.logLevel = LogLevel::Quiet;
            }
          },
          "Suppress non-error output")
      ->group("Global Options");

  // 输出文件
  app_.add_option("-o,--output", opts.output.file, "Output file path")
      ->group("Output Options");

  // 输出格式
  app_.add_option("-f,--format", opts.output.format,
                  "Output format (text, json)")
      ->transform(CLI::CheckedTransformer(
          std::map<std::string, OutputFormat>{{"text", OutputFormat::Text},
                                              {"json", OutputFormat::Json}},
          CLI::ignore_case))
      ->group("Output Options");

  // 禁用颜色
  app_.add_flag(
          "--no-color",
          [&opts](std::int64_t count) {
            if (count > 0) {
              opts.global.colorDiagnostics = false;
            }
          },
          "Disable colored output")
      ->group("Global Options");
}

VoidResult Cli::loadConfig() {
  // TODO: 实现配置文件加载
  // 优先级: 命令行参数 > 项目配置文件 > 全局配置文件 > 默认值
  return ok();
}

} // namespace czc::cli
