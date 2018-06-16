```bash
cp deamon/agent.plist ~/Library/LaunchAgents/com.macbury.circleci.plist
nano ~/Library/LaunchAgents/com.macbury.circleci.plis
launchctl load -w ~/Library/LaunchAgents/com.macbury.circleci.plist
launchctl unload -w ~/Library/LaunchAgents/com.macbury.circleci.plist
```
