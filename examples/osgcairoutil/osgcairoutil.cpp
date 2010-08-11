#include <osg/Math>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgCairo/Image>
#include <osgCairo/Util>

void drawPie(cairo_t* c, int w, int h) {
	double w2  = w / 2.0f;
	double h2  = h / 2.0f;
	double r   = h2 * 0.8f;
	double seg = (2.0f * osg::PI) / 8.0f;
	double top = osg::PI + (osg::PI / 2.0f);
	
	// each "tier" or level or an arc
	double lvl[] = { 0.0f, 0.25f, 0.50f, 0.75f, 1.0f };

	const char* atts[] = {
		"Milk",
		"Horns",
		"Beefiness",
		"Moo/HR",
		"Grazing",
		"Sleeping",
		"Cowpower!",
		"s p o t s"
	};
	
	double attl[] = {
		0.25f,
		0.0f,
		0.50f,
		1.0f,
		0.0f,
		0.25f,
		0.75f,
		1.0f
	};

	// cairo_push_group(c);

	cairo_set_line_width(c, ((w + h) / 2.0f) * 0.003f);
	cairo_select_font_face(c, "SegoeUI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(c, ((w + h) / 2.0f) * 0.05f);
        
	cairo_translate(c, w2, h2);
	cairo_rotate(c, -seg / 2.0f);
	cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f);

	for(unsigned int i = 0; i < 8; i++) {
		// Pick a random level...
		cairo_move_to(c, 0.0f, 0.0f);
		cairo_line_to(c, 0.0f, -r);
        	cairo_arc(c, 0.0f, 0.0f, attl[i] * r, top, top + seg);
		cairo_line_to(c, 0.0f, 0.0f);
		cairo_set_source_rgba(c, 0.2f, 0.8f, 0.2f, 0.5f);
		cairo_fill(c);
		cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f);
	
        	// Do the various levels...
		cairo_move_to(c, 0.0f, 0.0f);
		cairo_line_to(c, 0.0f, -r);
		cairo_stroke(c);
		
		for(unsigned int l = 0; l < 5; l++) {
			cairo_arc(c, 0.0f, 0.0f, lvl[l] * r, top, top + seg);
			cairo_stroke(c);
		}

		cairo_text_extents_t extents;
		
		cairo_text_extents(c, atts[i], &extents);
		
		double arcsize = r * seg;
	
		// ------------------------------------
		cairo_save(c);
	
		double tr = extents.width / r;
		double aa = ((arcsize - extents.width) / 2.0f) / r;

		cairo_arc(c, 0.0f, 0.0f, h2 * 0.85f, top + aa, top + aa + tr);
		cairo_set_tolerance(c, 0.01f);

		cairo_path_t* path = cairo_copy_path_flat(c);

		cairo_new_path(c);
		cairo_text_path(c, atts[i]);

		osgCairo::util::mapPathOnto(c, path);
	
		cairo_path_destroy(path);

		cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f);
		cairo_set_line_width(c, 1.0f);
        	cairo_stroke_preserve(c);
		cairo_set_source_rgba(c, 0.8f, 0.5f, 0.1f, 0.7f);
		cairo_fill(c);
	
		cairo_restore(c);
		// ------------------------------------
	
        	// Pick a random level...
		cairo_move_to(c, 0.0f, 0.0f);
		cairo_line_to(c, 0.0f, -r);
		cairo_arc(c, 0.0f, 0.0f, r, top, top + seg);
		cairo_line_to(c, 0.0f, 0.0f);
		cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f);
		cairo_stroke(c);

		cairo_rotate(c, seg);
	}

	/*
	cairo_pattern_t* pat = osgCairo::util::displacedBlur(c, cairo_pop_group(c), 10);

	cairo_set_operator(c, CAIRO_OPERATOR_SOURCE);
	cairo_translate(c, -5, -5);
	cairo_set_source(c, pat);
	cairo_paint(c);

	cairo_pattern_destroy(pat);
	*/
}

osg::Geode* createExample(unsigned int size) {
	osg::Geode*      geode = new osg::Geode();
	osgCairo::Image* image = new osgCairo::Image();
	
	if(image->allocateSurface(size, size, CAIRO_FORMAT_ARGB32)) {
		osg::Texture2D* texture = new osg::Texture2D();
		osg::Geometry*  geom    = osg::createTexturedQuadGeometry(
			osg::Vec3(0.0f, 0.0f, 0.0f),
			osg::Vec3(image->s(), 0.0f, 0.0f),
			osg::Vec3(0.0f, image->t(), 0.0f),
			0.0f,
			0.0f, 
			1.0f,
			1.0f
		);
		
		cairo_t* c = image->createContext();

		if(!cairo_status(c)) {
			drawPie(c, image->s(), image->t());

			cairo_destroy(c);
		}

		texture->setImage(image);
		texture->setDataVariance(osg::Object::DYNAMIC);
        
		osg::StateSet* state = geom->getOrCreateStateSet();
        
		state->setTextureAttributeAndModes(
			0,
			texture,
			osg::StateAttribute::ON
		);
        
		state->setMode(GL_BLEND, osg::StateAttribute::ON);
		state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		state->setAttributeAndModes(
			new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE_MINUS_SRC_ALPHA)
		);

		image->dirty();

		geode->addDrawable(geom);
	}

	return geode;
}

osg::Matrix createInvertedYOrthoProjectionMatrix(float width, float height) {
	osg::Matrix m = osg::Matrix::ortho2D(0.0f, width, 0.0f, height);
	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -height, 0.0f);

	return t * s * m;
}

osg::Camera* createOrthoCamera(float width, float height) {
	osg::Camera* camera = new osg::Camera();

	camera->getOrCreateStateSet()->setMode(
		GL_LIGHTING,
		osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
	);

	camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width, 0.0f, height));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);

	return camera;
}

osg::Camera* createInvertedYOrthoCamera(float width, float height) {
	osg::Camera* camera = createOrthoCamera(width, height);

	camera->setProjectionMatrix(createInvertedYOrthoProjectionMatrix(width, height));

	return camera;
}

int main(int, char**) {
	osgViewer::Viewer viewer;

	unsigned int size = 512;

	osg::Camera* camera = createInvertedYOrthoCamera(size, size);
	osg::Group*  group  = new osg::Group();

	camera->addChild(createExample(size));

	group->addChild(camera);
	group->addChild(osgDB::readNodeFile("cow.osg"));

	viewer.setSceneData(group);
	viewer.setUpViewInWindow(50, 50, size, size);
	viewer.addEventHandler(new osgViewer::StatsHandler());

	return viewer.run();
}
