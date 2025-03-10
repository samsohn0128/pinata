package capstone.aiimageeditor.ui


import android.graphics.Bitmap
import android.graphics.Color
import android.os.AsyncTask
import android.os.Bundle
import android.util.Log
import android.view.*
import android.widget.ImageView
import android.widget.SeekBar
import androidx.fragment.app.Fragment
import capstone.aiimageeditor.ImageHalo
import capstone.aiimageeditor.ImageManager
import capstone.aiimageeditor.R
import capstone.aiimageeditor.customviews.LiquifyView
import capstone.aiimageeditor.customviews.TallView //add tallview
import capstone.aiimageeditor.imageprocessing.GPUImageFilterTools
import com.bumptech.glide.Glide
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.tabs.TabLayout
import jp.co.cyberagent.android.gpuimage.GPUImage
import jp.co.cyberagent.android.gpuimage.filter.GPUImageFilter
import kotlinx.android.synthetic.main.fragment_person.*
import org.opencv.android.Utils
import org.opencv.core.Mat
import java.lang.Exception
import yuku.ambilwarna.AmbilWarnaDialog

//밝기 내리는거, 대조 올리는거 잘됨
//밝기 올리는거, 대조 내리는거 잘 안됨
//톤은 컨트롤바가 안 나옴
//채도는 컨트롤바 만져도 안 바뀜
//틴트하면 배경이 까맣게 바뀌고 틴트를 최대치로 하면 인물도 까맣게 됨

class FragmentPerson : Fragment(), View.OnClickListener, View.OnTouchListener {

    private lateinit var seekBar: SeekBar
    private lateinit var imageBG: ImageView
    private lateinit var imageFG: ImageView
    private lateinit var imageLiquify: LiquifyView
    private lateinit var imageTall: TallView //add tallview
    private lateinit var gpuImage: GPUImage
    private lateinit var tabLayout: TabLayout
    private lateinit var imageManager: ImageManager
    private lateinit var imageHalo: ImageHalo
    private lateinit var buttonColorChange: FloatingActionButton
    private lateinit var buttonToggleLiquify: FloatingActionButton
    private var tabPosition = 0
    private var filterAdjuster: GPUImageFilterTools.FilterAdjuster? = null
    private var isLiquify = true


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

    }


    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        seekBar = view.findViewById(R.id.seekBar)
        imageBG = view.findViewById(R.id.image_bg)
        imageFG = view.findViewById(R.id.image_fg)
        tabLayout = view.findViewById(R.id.tabLayout)
        imageLiquify = view.findViewById(R.id.view_liquifyview)

        imageTall = view.findViewById(R.id.view_Tallview) //add tallview


        buttonColorChange = view.findViewById(R.id.buttonColorChange)
        buttonToggleLiquify = view.findViewById(R.id.buttonTogleLiquify)
        buttonColorChange.setOnClickListener(this)
        buttonTogleLiquify.setOnClickListener(this)


        imageManager = (activity?.application as ImageManager)


        gpuImage = GPUImage(context)

        imageFG.setOnTouchListener(this)


        buttonColorChange.visibility = View.GONE
        imageBG.visibility = View.VISIBLE
        seekBar.max = 100
        seekBar.visibility = View.GONE

        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {

                if (tabPosition == 0) {
                    if (isLiquify) imageLiquify.brushsizechange(progress / 25) //0~4
                    else imageTall.tall(progress.toFloat() / 33 / 10 + 1.0f) //1.00f ~ 1.30f

                } else if (tabPosition != 8) {
                    imageManager.personAdjusts[tabPosition] = progress
                    filterAdjuster = GPUImageFilterTools.FilterAdjuster(imageManager.personFilters[tabPosition]!!)
                    filterAdjuster?.adjust(progress)
                    applyFilters(true)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                if (tabPosition == 8) {
                    if (seekBar != null) {
                        if(seekBar.progress==0)imageManager.doHalo=false
                        else {
                            imageManager.doHalo=true
                            imageManager.personAdjusts[8]=seekBar.progress
                            imageHalo.setWeight(seekBar.progress)
                        }
                    }
                    applyFilters(true)
                }
            }
        })
        tabLayout.addOnTabSelectedListener(tabListener)
    }


    fun refreshBackground() {
        imageBG.setImageBitmap(gpuImage.getBitmapWithFiltersApplied(imageManager.backgroundOriginal,imageManager.backgroundFilters))
    }

    public fun setImage() {
        imageBG.setImageBitmap( imageManager.backgroundFiltered)
        imageLiquify.setup(30, 50, imageManager.personOriginal, imageManager.backgroundOriginal)
        imageTall.setup(1, 50, imageManager.personOriginal, imageManager.backgroundOriginal)
        imageHalo = ImageHalo()
        imageHalo.setWeight(imageManager.personAdjusts[8])
        imageHalo.setColor(imageManager.haloColor)
        imageLiquify.visibility = View.VISIBLE
        seekBar.visibility = View.VISIBLE
        imageFG.setImageBitmap(imageManager.personFiltered)
        tabLayout.getTabAt(0)?.select()
    }

    public fun saveImage() {
        if (tabPosition == 0) {
            if (isLiquify) imageManager.personOriginal = imageLiquify.getLiquifiedImage(imageManager.original.width, imageManager.original.height)
            else imageManager.personOriginal = imageTall.getTalledImage(imageManager.original.width, imageManager.original.height)

        }
        applyFilters(false)
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_person, container, false)
    }

    private fun addFilter(f: GPUImageFilter) {
        imageFG.visibility = View.VISIBLE
        val index = tabPosition
        var filter = f
        if (imageManager.personFilters[index] != null) {
            filter = imageManager.personFilters[index]!!
            seekBar.progress = imageManager.personAdjusts[index];
        } else {
            imageManager.personFilters[index] = f
        }

        filterAdjuster = GPUImageFilterTools.FilterAdjuster(filter)
        if (filterAdjuster!!.canAdjust()) {
            seekBar.visibility = View.VISIBLE
        } else {
            seekBar.visibility = View.GONE
        }
    }

    /*
    brightness 45~55
    contrast 40~60
    gamma 40~60
    sharpness 40~60
    saturation 0~100
    exposure 45~55
    highlight 0~50
    whitebalance 30~70
    vinette 50~70
    gausian blur -> 흐리
    Toon, SmoothToon -> 만화효과
    haze 40~60
    vibrance 25~75
     */
    val tabListener = object : TabLayout.OnTabSelectedListener {
        override fun onTabReselected(tab: TabLayout.Tab?) {
            when (tab?.position) {
            }
        }

        override fun onTabUnselected(tab: TabLayout.Tab?) {
            when (tab?.position) {
                0 -> {
                    if (isLiquify) {
                        imageLiquify.visibility = View.GONE
                        imageManager.personOriginal = imageLiquify.getLiquifiedImage(imageManager.original.width, imageManager.original.height)
                        buttonTogleLiquify.setImageResource(R.drawable.ic_finger)
                    } else {
                        imageTall.visibility = View.GONE

                        imageManager.personOriginal = imageTall.getTalledImage(
                            imageManager.original.width,
                            imageManager.original.height
                        )
                        buttonTogleLiquify.setImageResource(R.drawable.ic_height)
                    }
                    isLiquify = true
                }
            }
        }

        override fun onTabSelected(tab: TabLayout.Tab?) {
/*
            applyFilters(true)
*/

            buttonColorChange.visibility = View.GONE
            seekBar.visibility = View.VISIBLE
            tabPosition = tab!!.position
            imageLiquify.visibility = View.GONE
            imageTall.visibility = View.GONE
            imageFG.visibility = View.GONE
            white_view.visibility = View.GONE
            buttonTogleLiquify.visibility = View.GONE

            when (tab?.position) {
                0 -> {
                    buttonTogleLiquify.visibility = View.VISIBLE
                    setLiquify()
                }
                1 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.GAMMA))
                2 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.SATURATION))
                3 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.EXPOSURE))
                4 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.HIGHLIGHT_SHADOW))
                5 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.WHITE_BALANCE))
                6 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.HAZE))
                7 -> addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.VIBRANCE))
                8 -> {
                    imageFG.visibility = View.VISIBLE
                    buttonColorChange.visibility = View.VISIBLE
                    imageHalo.setColor(imageManager.haloColor)
                    seekBar.progress = 0
                    applyFilters(true)
                }
                9 -> {
                    addFilter(GPUImageFilterTools.createFilterForType(context!!, GPUImageFilterTools.FilterType.TOON))
                }
            }
            seekBar.progress = imageManager.personAdjusts[tabPosition]
        }

    }

    fun applyFilters(toImageView: Boolean) {
        var bitmap = Bitmap.createBitmap(imageManager.personOriginal)
        bitmap = gpuImage.getBitmapWithFiltersApplied(bitmap, imageManager.personFilters)
        Log.i("!!","${imageManager.doHalo}")
        if (imageManager.doHalo) setImage(toImageView, imageHalo.run(bitmap))
        else {
            setImage(toImageView, bitmap)
        }

    }

    private fun setImage(toImageView: Boolean, bitmap: Bitmap) {
        if (toImageView) imageFG.setImageBitmap(bitmap)
        imageManager.personFiltered = bitmap
    }

    override fun onClick(p0: View?) {
        if (tabPosition == 0) {
            if (isLiquify) {
                imageLiquify.visibility = View.GONE
                imageManager.personOriginal = imageLiquify.getLiquifiedImage(imageManager.original.width, imageManager.original.height)
                buttonTogleLiquify.setImageResource(R.drawable.ic_finger)
            } else {
                //imageTall.removeLines()
                imageTall.visibility = View.GONE

                imageManager.personOriginal = imageTall.getTalledImage(
                    imageManager.original.width,
                    imageManager.original.height
                )
                buttonTogleLiquify.setImageResource(R.drawable.ic_height)
            }
            isLiquify = !isLiquify
            setLiquify()
        } else {
            val colorPicker = AmbilWarnaDialog(view?.context, Color.RED, true, object : AmbilWarnaDialog.OnAmbilWarnaListener {
                override fun onCancel(dialog: AmbilWarnaDialog?) {
                    return
                }

                override fun onOk(dialog: AmbilWarnaDialog?, color: Int) {
                    imageHalo.setColor(color)
                    applyFilters(true)
                }
            })
            colorPicker.show()
        }

    }

    override fun onTouch(p0: View?, p1: MotionEvent?): Boolean {
        when (p1?.action) {
            MotionEvent.ACTION_DOWN -> white_view.visibility = View.VISIBLE
            MotionEvent.ACTION_UP -> white_view.visibility = View.GONE
        }
        return true
    }

    fun setLiquify() {
        imageLiquify.visibility = View.GONE
        imageTall.visibility = View.GONE
        imageFG.visibility = View.GONE
        white_view.visibility = View.GONE
        if (isLiquify) {
            imageLiquify.setup(30, 50, imageManager.personOriginal, imageManager.backgroundOriginal)
            imageLiquify.visibility = View.VISIBLE
            seekBar.progress = 0
            seekBar.visibility = View.VISIBLE
            white_view.visibility = View.VISIBLE
        } else {
            imageTall.setup(1, 50, imageManager.personOriginal, imageManager.backgroundOriginal)
            imageTall.visibility = View.VISIBLE
            white_view.visibility = View.VISIBLE

            seekBar.progress = 0
        }
    }
}