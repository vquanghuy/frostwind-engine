import CoreGraphics
import Foundation

// Select only the app's visible normal window, never an entire desktop.
let requestedPID = CommandLine.arguments.count > 1 ? Int(CommandLine.arguments[1]) : nil
let options: CGWindowListOption = [.optionOnScreenOnly, .excludeDesktopElements]
guard let windows = CGWindowListCopyWindowInfo(options, kCGNullWindowID) as? [[String: Any]] else {
    fputs("Unable to read the window list.\n", stderr)
    exit(1)
}

let matches = windows.filter { window in
    guard let owner = window[kCGWindowOwnerName as String] as? String,
          owner.lowercased() == "mini-artillery",
          let layer = window[kCGWindowLayer as String] as? Int,
          layer == 0 else { return false }
    if let pid = requestedPID {
        return window[kCGWindowOwnerPID as String] as? Int == pid
    }
    return true
}.sorted {
    ($0[kCGWindowOwnerPID as String] as? Int ?? 0) >
    ($1[kCGWindowOwnerPID as String] as? Int ?? 0)
}

guard let selected = matches.first,
      let id = selected[kCGWindowNumber as String] as? Int else {
    fputs("No visible Mini Artillery window found. Launch or restore the app first.\n", stderr)
    exit(1)
}
print(id)
