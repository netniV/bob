import Foundation
import SwiftUI

struct ActionView: View, XSollaUpdaterDelegate {

  @StateObject var gameUpdater = GameUpdaterViewModel()
  @State private var gameVersion: Int = 0
  @State private var gameUpdateAvailable: Bool = false
  @State private var updating: Bool = false
  @State private var updateAction: String = ""
  @State private var updateSubAction: String = ""
  @State private var updateProgress: Float = 0.0
  @State private var gameInstalled: Bool = false
  @State private var gameRunning: Bool = false
  private var model = PulsatingViewModel()

  func updateProgress(progress: XsollaUpdateProgress) {
    switch progress {
    case .Start(_):
      break
    case .Progress(let currentAction, let totalActions):
      updateAction = "Updating \(currentAction) of \(totalActions)"
      updateProgress = Float(currentAction) / Float(totalActions)
      break
    case .Extracting(_):
      updateSubAction = "Extracting"
      break
    case .ExtractComplete(_):
      break
    case .Downloading(_):
      updateSubAction = "Downloading"
      break
    case .DownloadComplete(_):
      break
    case .Patching(_):
      updateSubAction = "Patching"
      break
    case .PatchingProgress(_, _):
      break
    case .PatchStepComplete:
      break
    case .PatchComplete:
      break
    case .Waiting:
      updateSubAction = "Waiting"
      break
    case .ApplyVersion:
      break
    case .VersionApplied:
      break
    case .Finalizing:
      updateSubAction = "Finalizing"
      break
    case .CleaningUp:
      updateSubAction = "Cleaning Up"
      break
    case .Complete:
      updateSubAction = "Complete"
      break
    }
  }

  var body: some View {
    GeometryReader { geo in
      Grid {
        GridRow {
          if gameInstalled {
            Button {
              withAnimation {
                openSettings()
              }
            } label: {
              commonButton(text: "Open Settings")
                .foregroundColor(.lcarViolet)
            }.buttonStyle(PlainButtonStyle())
            Button {
              withAnimation {
                if gameUpdateAvailable && !updating {
                  Task {
                    do {
                      updating = true
                      updateAction = "Starting"
                      updateSubAction = "Planning"
                      try await gameUpdater.updateGame(delegate: self)
                    } catch {
                      print("Error updating game: \(error)")
                    }
                    updating = false
                    gameVersion = gameUpdater.getInstalledGameVersion()
                    gameUpdateAvailable = await gameUpdater.checkForGameUpdate()
                  }
                }
              }
            } label: {
              if gameUpdateAvailable {
                commonButton(text: "Update Game!")
                  .foregroundColor(.lcarTan)
              } else {
                commonButton()
                  .foregroundColor(.lcarTan)
              }
            }.buttonStyle(PlainButtonStyle())
            Button {
              withAnimation {
                launchGame()
              }
            } label: {
              commonButton(text: "Engage!")
                .foregroundColor(.lcarOrange)
            }.buttonStyle(PlainButtonStyle())
          } else {
            Text("Game not installed")
              .font(.custom("HelveticaNeue-CondensedBold", size: 40))
              .foregroundColor(.lcarTan)
              .offset(x: -45)
            //Button {
            //  withAnimation {
            //  }
            //} label: {
            //  commonButton(text: "Install Game!")
            //    .foregroundColor(.lcarTan)
            //}.buttonStyle(PlainButtonStyle())
          }

        }
        .opacity(updating || gameRunning ? 0.5 : 1.0)
        .allowsHitTesting(!updating && !gameRunning)
      }
      .frame(width: geo.size.width, height: 150)
      .offset(x: 85, y: 20)
      .task {
        repeat {
          gameVersion = gameUpdater.getInstalledGameVersion()
          gameInstalled = gameVersion > 0
          gameVersion = gameUpdater.getInstalledGameVersion()
          gameUpdateAvailable = await gameUpdater.checkForGameUpdate()
          try? await Task.sleep(for: .seconds(60))
        } while !Task.isCancelled

      }
      .overlay(alignment: .bottomTrailing) {
        Text("Game Version: \(String(format: "%02d", gameVersion))")
          .font(.custom("HelveticaNeue-CondensedBold", size: 17))
          .foregroundColor(.lcarTan)
          .offset(x: -10)
      }
      .overlay(alignment: .bottomLeading) {
        if updating {
          HStack {
            PulsatingView(viewModel: model)
            Text("\(updateAction) (\(updateSubAction))")
              .font(.custom("HelveticaNeue-CondensedBold", size: 17))
              .foregroundColor(.lcarTan)
              .offset(x: -10)
          }
          .offset(x: 115, y: 10)
        }
      }
    }
  }

  private func commonButton(text: String = "") -> some View {
    RoundedRectangle(cornerRadius: 20)
      .frame(width: 125, height: 50)
      .overlay(alignment: .bottomTrailing) {
        HStack {
          Spacer()
          Text(text.count > 0 ? text : "\(randomDigits(4))-\(randomDigits(3))")
            .font(.custom("HelveticaNeue-CondensedBold", size: 17))
            .foregroundColor(.black)
        }
        .scaleEffect(x: 0.7, anchor: .trailing)
        .padding(.bottom, 5)
        .padding(.trailing, 20)
      }
  }

  private func randomDigits(_ count: Int) -> String {
    (1...count)
      .map { _ in "\(Int.random(in: 0...9))" }
      .joined()
  }

  private func openSettings() {
    let library = FileManager.default.urls(for: .libraryDirectory, in: .userDomainMask).first
    if let library {
      let preferences = library.appendingPathComponent("Preferences").appendingPathComponent(
        "com.tashcan.startrekpatch")
      let settingsTomlPath = preferences.appendingPathComponent("community_patch_settings.toml")
      if !FileManager.default.fileExists(atPath: settingsTomlPath.path) {
        do {
          try "".write(to: settingsTomlPath, atomically: true, encoding: .utf8)
        } catch {}
      }
      NSWorkspace.shared.open(settingsTomlPath)
    }
  }

  private func launchGame() {
    DispatchQueue.global().async {
      DispatchQueue.main.async {
        gameRunning = true
      }
      let process = Process()
      let helper = Bundle.main.path(forAuxiliaryExecutable: "stfc-community-patch-loader")
      process.executableURL = URL(fileURLWithPath: helper!)
      DispatchQueue.global().async {
        do {
          try process.run()
        } catch {
          DispatchQueue.main.async {
            gameRunning = false
          }
          return
        }
        process.waitUntilExit()
        DispatchQueue.main.async {
          gameRunning = false
        }
      }
    }
  }
}

class PulsatingViewModel: ObservableObject {
  @Published var colorIndex = 1
}

struct PulsatingView: View {

  @ObservedObject var viewModel: PulsatingViewModel

  func colourToShow() -> Color {
    switch viewModel.colorIndex {
    case 0:
      return Color.lcarOrange
    case 1:
      return Color.lcarTan
    case 2:
      return Color.lcarViolet
    default:
      return Color.lcarPink
    }
  }

  @State var animate = false
  var body: some View {
    VStack {
      ZStack {
        Circle().fill(colourToShow().opacity(0.25)).frame(width: 40, height: 40)
          .scaleEffect(
            self.animate ? 1 : 0)
        Circle().fill(colourToShow().opacity(0.35)).frame(width: 30, height: 30)
          .scaleEffect(
            self.animate ? 1 : 0)
        Circle().fill(colourToShow().opacity(0.45)).frame(width: 15, height: 15)
          .scaleEffect(
            self.animate ? 1 : 0)
        Circle().fill(colourToShow()).frame(width: 16.25, height: 16.25)
      }
      .onAppear { self.animate = true }
      .animation(
        animate
          ? Animation.easeInOut(duration: 1.5).repeatForever(autoreverses: true)
          : .default,
        value: animate
      )
      .onChange(of: viewModel.colorIndex) { _ in
        self.animate = false
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) {
          self.animate = true
        }
      }

    }
  }
}
