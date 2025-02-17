/*
 * Copyright (c) 2023, Gregory Bertilson <zaggy1024@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "PlaybackStream.h"

#include <AK/Platform.h>
#include <LibCore/ThreadedPromise.h>

#if defined(AK_OS_SERENITY)
#    include <LibAudio/PlaybackStreamSerenity.h>
#endif

#if defined(HAVE_PULSEAUDIO)
#    include <LibAudio/PlaybackStreamPulseAudio.h>
#endif

namespace Audio {

#define TRY_OR_REJECT_AND_STOP(expression, promise)                \
    ({                                                             \
        auto&& __temporary_result = (expression);                  \
        if (__temporary_result.is_error()) [[unlikely]] {          \
            (promise)->reject(__temporary_result.release_error()); \
            return 1;                                              \
        }                                                          \
        __temporary_result.release_value();                        \
    })

ErrorOr<NonnullRefPtr<PlaybackStream>> PlaybackStream::create(OutputState initial_output_state, u32 sample_rate, u8 channels, u32 target_latency_ms, AudioDataRequestCallback&& data_request_callback)
{
    VERIFY(data_request_callback);
    // Create the platform-specific implementation for this stream.
#if defined(AK_OS_SERENITY)
    return PlaybackStreamSerenity::create(initial_output_state, sample_rate, channels, target_latency_ms, move(data_request_callback));
#elif defined(HAVE_PULSEAUDIO)
    return PlaybackStreamPulseAudio::create(initial_output_state, sample_rate, channels, target_latency_ms, move(data_request_callback));
#else
    (void)initial_output_state, (void)sample_rate, (void)channels, (void)target_latency_ms;
    return Error::from_string_literal("Audio output is not available for this platform");
#endif
}

}
