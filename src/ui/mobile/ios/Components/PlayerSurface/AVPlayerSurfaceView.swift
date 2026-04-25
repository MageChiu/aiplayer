import AVKit
import SwiftUI

struct AVPlayerSurfaceView: View {
    let player: AVPlayer

    var body: some View {
        VideoPlayer(player: player)
            .background(ColorTokens.playerBackground)
            .clipShape(RoundedRectangle(cornerRadius: 28, style: .continuous))
    }
}
