/**
 * @file diagnostics.hpp
 * @brief 诊断系统定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   定义编译器诊断系统：
 *   - DiagnosticLevel: 诊断级别
 *   - Diagnostic: 诊断信息
 *   - DiagnosticsEngine: 诊断引擎
 */

#ifndef CZC_COMMON_DIAGNOSTICS_HPP
#define CZC_COMMON_DIAGNOSTICS_HPP

#include "czc/common/config.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace czc {

/**
 * @brief 诊断级别枚举。
 */
enum class DiagnosticLevel : std::uint8_t {
  Note,    ///< 提示信息
  Warning, ///< 警告
  Error,   ///< 错误
  Fatal    ///< 致命错误
};

/**
 * @brief 诊断信息结构。
 */
struct Diagnostic {
  DiagnosticLevel level{DiagnosticLevel::Error}; ///< 诊断级别
  std::string message;                           ///< 诊断消息
  std::string code;                              ///< 错误码，如 "E001"
  std::string filename;                          ///< 源文件名
  std::uint32_t line{0};                         ///< 行号（1-based）
  std::uint32_t column{0};                       ///< 列号（1-based）

  /**
   * @brief 格式化诊断信息。
   *
   * @return 格式化后的字符串
   */
  [[nodiscard]] std::string format() const {
    std::string result;

    // 文件位置
    if (!filename.empty()) {
      result += filename;
      if (line > 0) {
        result += ":" + std::to_string(line);
        if (column > 0) {
          result += ":" + std::to_string(column);
        }
      }
      result += ": ";
    }

    // 诊断级别
    switch (level) {
    case DiagnosticLevel::Note:
      result += "note: ";
      break;
    case DiagnosticLevel::Warning:
      result += "warning: ";
      break;
    case DiagnosticLevel::Error:
      result += "error: ";
      break;
    case DiagnosticLevel::Fatal:
      result += "fatal error: ";
      break;
    }

    // 错误码和消息
    if (!code.empty()) {
      result += "[" + code + "] ";
    }
    result += message;

    return result;
  }
};

/**
 * @brief 诊断处理回调类型。
 */
using DiagnosticHandler = std::function<void(const Diagnostic &)>;

/**
 * @brief 诊断引擎，管理编译过程中的诊断信息。
 *
 * @details
 *   诊断引擎负责：
 *   - 收集和存储诊断信息
 *   - 统计错误和警告数量
 *   - 支持自定义诊断处理回调
 *
 *   设计参考 LLVM DiagnosticsEngine，但简化以适应项目规模。
 */
class DiagnosticsEngine {
public:
  DiagnosticsEngine() = default;
  ~DiagnosticsEngine() = default;

  // 不可拷贝
  DiagnosticsEngine(const DiagnosticsEngine &) = delete;
  DiagnosticsEngine &operator=(const DiagnosticsEngine &) = delete;

  // 可移动
  DiagnosticsEngine(DiagnosticsEngine &&) noexcept = default;
  DiagnosticsEngine &operator=(DiagnosticsEngine &&) noexcept = default;

  /**
   * @brief 报告诊断信息。
   *
   * @param diag 诊断信息
   */
  void report(Diagnostic diag) {
    // 更新统计
    switch (diag.level) {
    case DiagnosticLevel::Note:
      break;
    case DiagnosticLevel::Warning:
      ++warningCount_;
      break;
    case DiagnosticLevel::Error:
      ++errorCount_;
      break;
    case DiagnosticLevel::Fatal:
      ++errorCount_;
      hadFatalError_ = true;
      break;
    }

    // 调用处理回调
    if (handler_) {
      handler_(diag);
    }

    // 存储诊断
    diagnostics_.push_back(std::move(diag));
  }

  /**
   * @brief 报告错误。
   *
   * @param message 错误消息
   * @param code 错误码
   * @param filename 文件名
   * @param line 行号
   * @param column 列号
   */
  void error(std::string_view message, std::string_view code = "",
             std::string_view filename = "", std::uint32_t line = 0,
             std::uint32_t column = 0) {
    report(Diagnostic{
        .level = DiagnosticLevel::Error,
        .message = std::string(message),
        .code = std::string(code),
        .filename = std::string(filename),
        .line = line,
        .column = column,
    });
  }

  /**
   * @brief 报告警告。
   *
   * @param message 警告消息
   * @param code 警告码
   * @param filename 文件名
   * @param line 行号
   * @param column 列号
   */
  void warning(std::string_view message, std::string_view code = "",
               std::string_view filename = "", std::uint32_t line = 0,
               std::uint32_t column = 0) {
    report(Diagnostic{
        .level = DiagnosticLevel::Warning,
        .message = std::string(message),
        .code = std::string(code),
        .filename = std::string(filename),
        .line = line,
        .column = column,
    });
  }

  /**
   * @brief 报告提示。
   *
   * @param message 提示消息
   */
  void note(std::string_view message) {
    report(Diagnostic{
        .level = DiagnosticLevel::Note,
        .message = std::string(message),
        .code = std::string{},
        .filename = std::string{},
    });
  }

  /**
   * @brief 设置诊断处理回调。
   *
   * @param handler 处理回调函数
   */
  void setHandler(DiagnosticHandler handler) { handler_ = std::move(handler); }

  /// 获取错误数量
  [[nodiscard]] std::size_t errorCount() const noexcept { return errorCount_; }

  /// 获取警告数量
  [[nodiscard]] std::size_t warningCount() const noexcept {
    return warningCount_;
  }

  /// 检查是否有错误
  [[nodiscard]] bool hasErrors() const noexcept { return errorCount_ > 0; }

  /// 检查是否有致命错误
  [[nodiscard]] bool hadFatalError() const noexcept { return hadFatalError_; }

  /// 获取所有诊断信息
  [[nodiscard]] const std::vector<Diagnostic> &diagnostics() const noexcept {
    return diagnostics_;
  }

  /// 清空诊断信息
  void clear() noexcept {
    diagnostics_.clear();
    errorCount_ = 0;
    warningCount_ = 0;
    hadFatalError_ = false;
  }

private:
  std::vector<Diagnostic> diagnostics_;
  DiagnosticHandler handler_;
  std::size_t errorCount_{0};
  std::size_t warningCount_{0};
  bool hadFatalError_{false};
};

} // namespace czc

#endif // CZC_COMMON_DIAGNOSTICS_HPP
