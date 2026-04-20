# Debug Session: open-video-crash
- **Status**: [OPEN]
- **Issue**: 打开本地视频文件后应用异常退出
- **Debug Server**: http://127.0.0.1:7777/event
- **Log File**: .dbg/trae-debug-log-open-video-crash.ndjson

## Reproduction Steps
1. 启动应用
2. 点击“打开视频”
3. 选择一个本地视频文件
4. 观察应用在加载时异常退出

## Hypotheses & Verification
| ID | Hypothesis | Likelihood | Effort | Evidence |
|----|------------|------------|--------|----------|
| A | 打开文件后触发本地离线翻译或模型加载，进程在 `llama/whisper` 路径崩溃 | High | Med | Confirmed (partial) |
| B | `loadFile()` 中音频抽取线程与 `mpv loadfile` 并发导致资源竞争或生命周期问题 | High | Med | Rejected |
| C | `mpv` 加载文件后的事件回调进入异常路径，Qt 槽中访问失效状态 | Med | Med | Rejected |
| D | UI 层从文件选择到播放器加载的参数/状态流转存在问题 | Med | Low | Rejected |

## Log Evidence
- Debug server logs were not captured because the local debug server process had exited before reproduction.
- macOS crash report used instead:
  - `~/Library/Logs/DiagnosticReports/aiplayer-2026-04-18-110650.ips`
  - Exception: `EXC_CRASH / SIGABRT`
  - Faulting thread: `29`
  - Stack:
    - `libggml-base.0.dylib :: ggml_abort`
    - `libggml-base.0.dylib :: ggml_backend_dev_backend_reg`
    - `libwhisper.1.dylib :: whisper_init_with_params_no_state`
    - `libwhisper.1.dylib :: whisper_init_from_file_with_params`
    - `aiplayer :: MpvWidget::runWhisper()`
- This shows the abort happens while opening a file triggers ASR initialization, not in the file dialog/UI path and not in the immediate `mpv loadfile` path.

## Verification Conclusion
- Root cause is highly likely a runtime `ggml` ABI / symbol conflict introduced by loading both `whisper.cpp` and `llama.cpp` stacks into the same process with overlapping `ggml` dynamic libraries.
- The crash is reproducible when opening a local video because that path auto-starts `MpvWidget::runWhisper()`, which aborts inside `ggml` backend registration.
