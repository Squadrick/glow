// Copyright 2004-present Facebook. All Rights Reserved.
#ifndef GLOW_TORCH_GLOW_BACKEND_H
#define GLOW_TORCH_GLOW_BACKEND_H

#include "CachingGraphRunner.h"
#include "PyTorchCommon.h"
#include <torch/csrc/jit/backends/backend.h>
#include <torch/csrc/jit/backends/backend_interface.h>

namespace glow {

/// Debug Only: A graph runner used to run JIT graph via GraphExecutor.
/// This is used to run a mixed graph, containing PyTorch nodes and Glow Fusion
/// nodes.
class JITGraphRunner {
public:
  JITGraphRunner(c10::IValue module, std::shared_ptr<torch::jit::Graph> graph,
                 PyTorchLoaderSettings settings);

  torch::jit::Stack onExecute(c10::impl::GenericList inputs);

  int countFusionNodes();

private:
  /// Debug flow:
  /// PyTorch JIT graph's first input is the module. Store processed module and
  /// push it to stack when running JITGraphRunner on debug flow.
  c10::IValue module_;
  std::shared_ptr<torch::jit::Graph> graph_;
  torch::jit::GraphExecutor ptGraphExecutor_;
  PyTorchLoaderSettings settings_;
};

// Glow backend implementation to PyTorch backend
class TorchGlowBackend : public torch::jit::PyTorchBackendInterface {
public:
  TorchGlowBackend() {}
  ~TorchGlowBackend() override {}

  bool is_available() override;

  c10::impl::GenericDict
  compile(c10::IValue processed,
          c10::impl::GenericDict method_compile_spec) override;

  c10::impl::GenericList execute(c10::IValue handle,
                                 c10::impl::GenericList inputs) override;

  static void preview(torch::jit::Module mod);

private:
  std::unordered_map<int64_t, std::pair<std::unique_ptr<CachingGraphRunner>,
                                        std::unique_ptr<JITGraphRunner>>>
      handleToRunnerMap_;

  // Number of runs that have failed, used for dumping io files on failures.
  std::atomic<size_t> failedRunNum_{0};
};

/// Registers TorchGlowBackend, related custom classes and helper JIT IR ops.
void registerTorchGlowBackendAndDeps();

/// TorchGlowBackend preprocessing.
c10::IValue
preprocess(const torch::jit::Module &mod,
           const c10::Dict<c10::IValue, c10::IValue> &method_compile_spec);

// Register TorchGlowBackend
torch::jit::backend<TorchGlowBackend> &torchGlowBackend();

} // namespace glow
#endif // GLOW_TORCH_GLOW_BACKEND_H
