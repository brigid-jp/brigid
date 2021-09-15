## GitHub fork

https://help.github.com/ja/github/collaborating-with-issues-and-pull-requests/configuring-a-remote-for-a-fork

```
git remote add upstream https://github.com/love2d/love-android.git
```

```
git remote -v
origin	brigid.github.com:brigid-jp/love-android.git (fetch)
origin	brigid.github.com:brigid-jp/love-android.git (push)
upstream	https://github.com/love2d/love-android.git (fetch)
upstream	https://github.com/love2d/love-android.git (push)
```

https://help.github.com/ja/github/collaborating-with-issues-and-pull-requests/syncing-a-fork

```
git fetch upstream
remote: Enumerating objects: 4501, done.
remote: Counting objects: 100% (4182/4182), done.
remote: Compressing objects: 100% (1449/1449), done.
remote: Total 4501 (delta 2578), reused 4086 (delta 2534), pack-reused 319
Receiving objects: 100% (4501/4501), 9.23 MiB | 2.20 MiB/s, done.
Resolving deltas: 100% (2582/2582), completed with 1177 local objects.
From https://github.com/love2d/love-android
 * [new branch]        0.10.x          -> upstream/0.10.x
 * [new branch]        0.11.x          -> upstream/0.11.x
 * [new branch]        development     -> upstream/development
 * [new branch]        http_downloader -> upstream/http_downloader
 * [new branch]        master          -> upstream/master
 * [new branch]        remove-libs     -> upstream/remove-libs
```

## edit

```
git clone brigid.github.com:brigid-jp/love-android.git
cd love-android
git config user.name dev
git config user.email dev@brigid.jp
git checkout 11.3
git checkout -b 11.3-develop
git-checkout-feature
# edit...
git checkout 11.3-develop
git merge --no-ff -m "作業ブランチをマージ。" feature
git branch -d feature
git-push-all-tags
```

## import

```
cd love/platform/xcode/ios
mv ~/Workspace/xcode/love-11.3-ios-libraries/* .
```

## check

```
diff -qr love love-11.3-ios-source
```

## build

```
open love/platform/xcode/love.xcodeproj
```

## sync

```
git checkout 11.3-develop
git stash
git checkout master
git-clean -f
rm -fr src/libraries/brigid
git merge upstream/master
```

minorの初回チェックアウトとマージ。

```
git checkout -b minor origin/minor
git merge upstream/minor
```
