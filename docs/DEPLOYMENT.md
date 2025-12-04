# Deployment Guide

## Deploying to Vercel

### Prerequisites
- Vercel account (free): https://vercel.com/signup
- Git repository (optional but recommended)

### Option 1: Deploy via Vercel CLI (Recommended)

1. **Install Vercel CLI**
   ```bash
   npm install -g vercel
   ```

2. **Login to Vercel**
   ```bash
   vercel login
   ```

3. **Deploy**
   ```bash
   vercel
   ```
   
   Follow the prompts:
   - Set up and deploy? Yes
   - Which scope? Select your account
   - Link to existing project? No
   - What's your project's name? esp32-admin (or your choice)
   - In which directory is your code located? ./
   - Want to override settings? No

4. **Production Deployment**
   ```bash
   vercel --prod
   ```

### Option 2: Deploy via Vercel Dashboard

1. **Push to GitHub**
   ```bash
   git init
   git add .
   git commit -m "Initial commit"
   git remote add origin <your-github-repo-url>
   git push -u origin main
   ```

2. **Import Project**
   - Go to https://vercel.com/new
   - Click "Import Git Repository"
   - Select your repository
   - Click "Deploy"

### Option 3: Deploy via Drag & Drop

1. Build the project locally:
   ```bash
   pnpm build
   ```

2. Go to https://vercel.com/new

3. Drag and drop the `dist` folder

## Vercel Configuration

The project is already configured with `vite.config.js`. No additional setup needed!

### Build Settings
- **Build Command**: `pnpm build` or `npm run build`
- **Output Directory**: `dist`
- **Install Command**: `pnpm install` or `npm install`

### Environment Variables
None required for basic operation.

## Testing Deployment

After deployment, you'll receive a URL like:
```
https://your-project-name.vercel.app
```

### Verify Web Bluetooth Support
1. Open the URL in Chrome, Edge, or Opera
2. Open DevTools Console (F12)
3. Type: `navigator.bluetooth`
4. Should return a Bluetooth object (not undefined)

### Test on Mobile
1. Open URL on Android Chrome
2. Ensure HTTPS is used (Vercel provides this automatically)
3. Web Bluetooth will work on mobile

## Custom Domain (Optional)

1. Go to your project settings on Vercel
2. Navigate to "Domains"
3. Add your custom domain
4. Update DNS records as instructed

## Important Notes

### HTTPS Requirement
- Web Bluetooth requires HTTPS
- Vercel provides HTTPS automatically
- Do NOT use HTTP-only hosting

### Browser Support
✅ **Supported**:
- Chrome (Desktop & Android)
- Edge (Desktop & Android)
- Opera (Desktop & Android)

❌ **Not Supported**:
- Firefox (no Web Bluetooth)
- Safari / iOS (no Web Bluetooth)
- Internet Explorer

### CORS and Security
- No CORS issues since admin panel only connects to ESP32 via Bluetooth
- No backend API needed
- All communication is client-side

## Troubleshooting

### "Web Bluetooth API is not available"
- Ensure you're using HTTPS
- Check browser compatibility
- Try Chrome/Edge if using unsupported browser

### "Deploy Failed"
- Check build logs in Vercel dashboard
- Ensure all dependencies are in package.json
- Try `pnpm install && pnpm build` locally first

### "Cannot connect to device"
- Ensure ESP32 is powered and advertising
- Check that you're using a supported browser
- Bluetooth must be enabled on your device

## Continuous Deployment

### Automatic Deployments
If using GitHub integration:
- Every push to `main` triggers production deployment
- Pull requests get preview deployments
- No manual deployment needed

### Manual Updates
Using Vercel CLI:
```bash
# Make changes to code
git add .
git commit -m "Update admin panel"
vercel --prod
```

## Monitoring

### View Logs
```bash
vercel logs <deployment-url>
```

### Analytics
- Enable in Vercel dashboard
- Track page views and performance
- Monitor Web Vitals

## Cost

Vercel free tier includes:
- ✅ Unlimited deployments
- ✅ 100GB bandwidth/month
- ✅ Automatic HTTPS
- ✅ Custom domains
- ✅ Preview deployments

This project easily fits within free tier limits.

## Next Steps

After deployment:
1. Share the URL with users
2. Bookmark it for easy access
3. Test the full flow: Connect → Configure → Upload → Deploy
4. Monitor Vercel analytics for usage

---

**Deployment Time**: ~2 minutes  
**Build Time**: ~30 seconds  
**No server required**: Static site only
