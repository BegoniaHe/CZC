/**
 * @file diag_context.cpp
 * @brief 诊断上下文实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/diag_context.hpp"
#include "czc/diag/emitter.hpp"

#include <mutex>
#include <set>

namespace czc::diag {

/// DiagContext 内部实现
struct DiagContext::Impl {
  std::unique_ptr<Emitter> emitter;
  const SourceLocator *locator{nullptr};
  DiagConfig config;

  // 统计数据
  size_t errorCount{0};
  size_t warningCount{0};
  size_t noteCount{0};
  bool hadFatal{false};
  std::set<ErrorCode> uniqueErrorCodes; ///< 唯一错误码集合

  // 去重（可选）
  std::set<std::string> seenDiagnostics;

  // 线程安全
  mutable std::mutex mutex;

  Impl(std::unique_ptr<Emitter> e, const SourceLocator *l, DiagConfig c)
      : emitter(std::move(e)), locator(l), config(std::move(c)) {}
};

DiagContext::DiagContext(std::unique_ptr<Emitter> emitter,
                         const SourceLocator *locator, DiagConfig config)
    : impl_(std::make_unique<Impl>(std::move(emitter), locator,
                                   std::move(config))) {}

DiagContext::~DiagContext() = default;

DiagContext::DiagContext(DiagContext &&) noexcept = default;
auto DiagContext::operator=(DiagContext &&) noexcept -> DiagContext & = default;

void DiagContext::emit(Diagnostic diag) {
  std::lock_guard lock(impl_->mutex);

  // 处理 -Werror
  if (impl_->config.treatWarningsAsErrors && diag.level == Level::Warning) {
    diag.level = Level::Error;
  }

  // 去重检查
  if (impl_->config.deduplicate) {
    std::string key = diag.message.markdown().data();
    if (diag.code) {
      key = diag.code->toString() + ":" + key;
    }
    auto primarySpan = diag.primarySpan();
    if (primarySpan) {
      key += ":" + std::to_string(primarySpan->fileId) + ":" +
             std::to_string(primarySpan->startOffset);
    }

    if (impl_->seenDiagnostics.contains(key)) {
      return;
    }
    impl_->seenDiagnostics.insert(key);
  }

  // 更新统计
  switch (diag.level) {
  case Level::Error:
  case Level::Bug:
    ++impl_->errorCount;
    if (diag.code) {
      impl_->uniqueErrorCodes.insert(*diag.code);
    }
    break;
  case Level::Fatal:
    ++impl_->errorCount;
    impl_->hadFatal = true;
    if (diag.code) {
      impl_->uniqueErrorCodes.insert(*diag.code);
    }
    break;
  case Level::Warning:
    ++impl_->warningCount;
    break;
  case Level::Note:
  case Level::Help:
    ++impl_->noteCount;
    break;
  default:
    break;
  }

  // 检查最大错误数
  if (impl_->config.maxErrors > 0 &&
      impl_->errorCount > impl_->config.maxErrors) {
    return;
  }

  // 发射
  if (impl_->emitter) {
    impl_->emitter->emit(diag, impl_->locator);
  }
}

auto DiagContext::emitError(Diagnostic diag) -> ErrorGuaranteed {
  if (diag.level < Level::Error) {
    diag.level = Level::Error;
  }
  emit(std::move(diag));
  return createErrorGuaranteed();
}

void DiagContext::emitWarning(Diagnostic diag) {
  diag.level = Level::Warning;
  emit(std::move(diag));
}

void DiagContext::emitNote(Diagnostic diag) {
  diag.level = Level::Note;
  emit(std::move(diag));
}

auto DiagContext::error(Message message) -> ErrorGuaranteed {
  return emitError(Diagnostic(Level::Error, std::move(message)));
}

auto DiagContext::error(ErrorCode code, Message message, Span span)
    -> ErrorGuaranteed {
  Diagnostic diag(Level::Error, std::move(message), code);
  diag.spans.addPrimary(span, "");
  return emitError(std::move(diag));
}

void DiagContext::warning(Message message) {
  emitWarning(Diagnostic(Level::Warning, std::move(message)));
}

void DiagContext::note(Message message) {
  emitNote(Diagnostic(Level::Note, std::move(message)));
}

auto DiagContext::errorCount() const noexcept -> size_t {
  std::lock_guard lock(impl_->mutex);
  return impl_->errorCount;
}

auto DiagContext::warningCount() const noexcept -> size_t {
  std::lock_guard lock(impl_->mutex);
  return impl_->warningCount;
}

auto DiagContext::hasErrors() const noexcept -> bool {
  std::lock_guard lock(impl_->mutex);
  return impl_->errorCount > 0;
}

auto DiagContext::shouldAbort() const noexcept -> bool {
  std::lock_guard lock(impl_->mutex);
  if (impl_->hadFatal) {
    return true;
  }
  if (impl_->config.maxErrors > 0 &&
      impl_->errorCount >= impl_->config.maxErrors) {
    return true;
  }
  return false;
}

auto DiagContext::stats() const noexcept -> DiagnosticStats {
  std::lock_guard lock(impl_->mutex);
  DiagnosticStats result;
  result.errorCount = impl_->errorCount;
  result.warningCount = impl_->warningCount;
  result.noteCount = impl_->noteCount;
  result.uniqueErrorCodes = impl_->uniqueErrorCodes;
  return result;
}

void DiagContext::emitSummary() {
  std::lock_guard lock(impl_->mutex);
  if (impl_->emitter) {
    DiagnosticStats s;
    s.errorCount = impl_->errorCount;
    s.warningCount = impl_->warningCount;
    s.noteCount = impl_->noteCount;
    s.uniqueErrorCodes = impl_->uniqueErrorCodes;
    impl_->emitter->emitSummary(s);
  }
}

void DiagContext::setLocator(const SourceLocator *locator) {
  std::lock_guard lock(impl_->mutex);
  impl_->locator = locator;
}

auto DiagContext::locator() const noexcept -> const SourceLocator * {
  return impl_->locator;
}

auto DiagContext::config() const noexcept -> const DiagConfig & {
  return impl_->config;
}

auto DiagContext::config() noexcept -> DiagConfig & { return impl_->config; }

void DiagContext::flush() {
  std::lock_guard lock(impl_->mutex);
  if (impl_->emitter) {
    impl_->emitter->flush();
  }
}

auto DiagContext::createErrorGuaranteed() -> ErrorGuaranteed {
  return ErrorGuaranteed();
}

} // namespace czc::diag
