## edit

```
git clone brigid.github.com:brigid-jp/love.git
cd love
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

## android


