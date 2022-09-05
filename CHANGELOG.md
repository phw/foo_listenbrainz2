## v1.3.1
- Fixed packaging not including all files (missing x64 binaries and license files), issue #4.

## v1.3.0
- Provide a unified package including 32 and 64 bit version of this component.
- Submit `duration` of listened audio.
- Replaced submission of non-standard `listening_from` with `media_player`, `media_player_version`, `submission_client` and `submission_client_version`.
- Add preference to disable submission of the above client details.
- The additional information is also submitted for "playing now" notifications.

## v1.2.0
- Support foobar2000 v2.0 dark mode
- 64-bit build for foobar2000 v2.0
- Fixed help URL in preferences dialog

## v1.1.3
- Add a new `Preference` for sending all `ARTIST` tags if they're multi-value. Previous versions always sent the first value only and this remains the current default.

## v1.1.2
- Add toggle to `Playback` menu to enable/disable submissions. It will only be available after a token has been set.
- Restore skip track by query option. Note that previous settings are not remembered and custom queries will need setting again.

## v1.1.1
- Removed option to skip submitting tracks by query.
- Tracks must be at least 5 seconds long to count.
- Add support for submitting work `MBID`s.
- Add support for submitting multi-value artist/work `MBID`s.
- Sets `listening_from` to `foobar2000`.

## v1.1.0
- Add caching support for when `ListenBrainz` is down or you are offline. It all happens automatically, check the `foobar2000` `Console` for messages.
- Use foobar2000 SDK http client and add better error reporting.

## v1.0.0
- Initial release.
